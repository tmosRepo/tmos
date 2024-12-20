/*
 * handle.cpp
 *
 *  Created on: 2010-4-9
 *      Author: stanly
 */

#include <tmos.h>

//*----------------------------------------------------------------------------
//*			Unsafe (this=NULL) methods
//*----------------------------------------------------------------------------


//----------------------    ctors etc   ---------------------------//
/**
 * CHandle constructor
 */
CHandle::CHandle(void)
{
	if(IS_NOT_NULL(this))
	{
		drv_index = INALID_DRV_INDX;
		res = RES_CLOSED;
	}
}

/**
 * CHandle constructor
 */
CHandle::CHandle(DRIVER_INDEX index, const void * m)
{
	if(IS_NOT_NULL(this))
	{
		drv_index = index;
		mode.as_voidptr = (void*)m;
		res = RES_CLOSED;
	}
}

/**
 * CCHandle destructor
 *
 */
CHandle::~CHandle()
{
	close();
}


//----------------------    common      ---------------------------//
/**
 * Close the handle
 */
void CHandle::close()
{
	if(IS_NOT_NULL(this))
	{
		if(res < RES_CLOSED)
		{
			if( client.drv_index > INALID_DRV_INDX && !(client.drv_index & 3))
			{
				//this is a task handle...

				//1. Cancel any operation in progress..
				tsk_cancel();

				//2. Close
		        tsk_hcontrol(DCR_CLOSE);

		        //3. Deallocate the signal
				CURRENT_TASK->aloc_sig &= ~signal;

				drv_index = INALID_DRV_INDX;
				res = RES_CLOSED;
			} else
			{
				//this is driver �r callback handle...
				if(res == RES_BUSY)
					hcontrol(DCR_CANCEL);
				if( res < RES_BUSY )
				{
					hcontrol(DCR_CLOSE);

					drv_index = INALID_DRV_INDX;
					res = RES_CLOSED;
				}

			}
		}
	}
}

/**
 * Open
 * @param index
 * @param m
 * @return
 */
bool CHandle::tsk_open(DRIVER_INDEX index, const void * m)
{
	if(IS_NOT_NULL(this))
	{
		unsigned int sig;
		if(index >= INALID_DRV_INDX)
		{
			TRACELN_ERROR("Handle has an invalid driver index:%x", this);
			return false;
		}
		error = 0;
		if( drv_index != INALID_DRV_INDX && !(res & FLG_CLOSED))
		{
			TRACELN_ERROR("Handle is already open:%x", this);
			close();
		}
	    drv_index = index;
	    mode.as_voidptr = (void*)m;
	    client.task = CURRENT_TASK;

	    sig = CURRENT_TASK->aloc_sig;
	    sig = (sig+1) & ~sig;
	    sig &= 255;
	    if( (signal = sig) )
	    {
		 	res = RES_CLOSED;
		 	tsk_hcontrol(DCR_OPEN);

			if(res == RES_OK)
			{
		    	CURRENT_TASK->aloc_sig |= signal;
				return (true);
			}
			TRACELN_ERROR("Handle open err INDEX:%u", drv_index);
	    } else
	    {
	    	TRACELN1_ERROR("\r\nNo free signals for task ");
	    	TRACE1_ERROR((char*)CURRENT_TASK->name);
	    }
	 	res = RES_CLOSED;
		drv_index = INALID_DRV_INDX;
	} else
		TRACELN_ERROR("Memory err INDEX:%u", index);
	return (false);
}

bool CHandle::drv_open(DRIVER_INDEX index, const void * m)
{
	if (IS_NOT_NULL(this))
	{
		error = 0;
		if ( drv_index != INALID_DRV_INDX && !(res & FLG_CLOSED))
		{
			TRACELN_ERROR("Handle is already open:%x", this);
			close();
		}
		drv_index = index;
		mode.as_voidptr = (void*) m;

		signal = DCR_SIGNAL;
		res = RES_CLOSED;
		hcontrol(DCR_OPEN);

		if (res == RES_OK)
		{
			return (true);
		}
		TRACELN_ERROR("Handle open err INDEX:%u", drv_index);
	 	res = RES_CLOSED;
		drv_index = INALID_DRV_INDX;
	}
	else
		TRACELN_ERROR("Memory err INDEX:%u", index);
	return (false);
}

void CHandle::drv_read(void * buf, unsigned int l)
{
	if(res < FLG_BUSY)
	{
		//handle is idle and open
		len = l;
		set_res_cmd(CMD_READ);
		dst.as_voidptr = buf;
		svc_drv_service(this);
	}
}

void CHandle::drv_write(const void * buf, unsigned int l)
{
	if(res < FLG_BUSY)
	{
		//handle is idle and open
		len = l;
		set_res_cmd(CMD_WRITE);
		src.as_cvoidptr = buf;
		svc_drv_service(this);
	}
}

void CHandle::drv_read_write(void *d, const void *s, unsigned int l)
{
	if(res < FLG_BUSY)
	{
		//handle is idle and open
		len = l;
		set_res_cmd(CMD_READ_WRITE);
		dst.as_voidptr = d;
		src.as_cvoidptr = s;
		svc_drv_service(this);
	}
}

/**
 * try to open in endless loop
 * @param index
 * @param m
 */
void CHandle::tsk_safe_open(DRIVER_INDEX index, const void * m)
{
	while(!tsk_open(index, m))
	{
		tsk_sleep(10);
	}
}

//*----------------------------------------------------------------------------
//*			Safe (this!=NULL) methods
//*		Call these methods only after successful open()
//*----------------------------------------------------------------------------

//----------------------    common      ---------------------------//

//----------------------   task related ---------------------------//
/**
 * Cancel the current operation (if any)
 */
void CHandle::tsk_cancel()
{
	if (res & FLG_BUSY)
	{
        tsk_hcontrol(DCR_CANCEL);
        tsk_get_signal(signal);
        res &= ~FLG_SIGNALED;
	} else
		if(res & FLG_SIGNALED)
		{
			tsk_get_signal(signal);
	        res &= ~FLG_SIGNALED;
		}
}

/**
 * Waits for the last transaction to complete
 * @return true if completed (or there is no transaction to complete)
 * 		   false if closed
 */
bool CHandle::complete()
{
	while (res >= FLG_SIGNALED)
	{
		if(res & (FLG_SIGNALED | FLG_BUSY) )
		{
			tsk_get_signal(signal);
	        res &= ~FLG_SIGNALED;
		} else
		{
			//handle is closed or with error
			return (false);
		}
	}
	return (true);
}



/**
 * Start read operation
 * @param buf
 * @param l
 * @return
 */
bool CHandle::tsk_start_read(void * buf, unsigned int l)
{
	if(!complete())
		return (false);

	//handle is idle and open
	len = l;
	set_res_cmd(CMD_READ);
	dst.as_voidptr = buf;
	tsk_start_handle();
	return (true);
}

/**
 * Blocking read
 * @param buf
 * @param l
 * @return
 */
RES_CODE CHandle::tsk_read(void * buf, unsigned int l)
{
	if(!complete())
		return (res);

	//handle is idle and open
	len = l;
	set_res_cmd(CMD_READ);
	dst.as_voidptr = buf;
	return (tsk_start_and_wait());
}

/**
 * Time limited read
 * @param buf
 * @param l
 * @param time
 * @return
 */
RES_CODE CHandle::tsk_read(void * buf, unsigned int l, unsigned int time)
{
	if(complete())
	{
		//handle is idle and open
		len = l;
		set_res_cmd(CMD_READ);
		dst.as_voidptr = buf;
	    tsk_start_handle();
	    if(tsk_wait_signal(signal, time))
	        res &= ~FLG_SIGNALED;
	    else
	    	tsk_cancel();
	}

   return (res);
}

RES_CODE CHandle::tsk_read_pkt(void * buf, unsigned int l, unsigned int time)
{
	unsigned int dwRead;

	tsk_read(buf, l, time);
	dwRead = l - len;
	l -= dwRead;
	while(l && dwRead && res <= RES_OK )
	{
		buf = (char*)buf + dwRead;
		tsk_resume_read(buf, l);
		dwRead = l - len;
		l -= dwRead;
	}
	if(!l && !res)
		res = RES_OK;
	return (res);
}

/**
 * Start read locked operation
 * @param buf
 * @param l
 * @return
 */
bool CHandle::tsk_start_read_locked(void * buf, unsigned int l)
{
	if(!complete())
		return (false);

	//handle is idle and open
	len = l;
	set_res_cmd(CMD_READ|CMD_LOCK);
	dst.as_voidptr = buf;
	tsk_start_handle();
	return (true);
}

/**
 * Blocking locked read
 * @param buf
 * @param l
 * @return
 */
RES_CODE CHandle::tsk_read_locked(void * buf, unsigned int l)
{
	if(!complete())
		return (res);

	//handle is idle and open
	len = l;
	set_res_cmd((CMD_READ|FLAG_LOCK));
	dst.as_voidptr = buf;
	return (tsk_start_and_wait());
}

/**
 * Time limited locked read
 * @param buf
 * @param l
 * @param time
 * @return
 */
RES_CODE CHandle::tsk_read_locked(void * buf, unsigned int l, unsigned int time)
{
	if(complete())
	{
		//handle is idle and open
		len = l;
		set_res_cmd((CMD_READ|FLAG_LOCK));
		dst.as_voidptr = buf;
	    tsk_start_handle();
	    if(tsk_wait_signal(signal, time))
	        res &= ~FLG_SIGNALED;
	    else
	    	tsk_cancel();
	}

    return (res);
}

/**
 * Resume read operation
 * @param buf
 * @param l
 * @return
 */
RES_CODE CHandle::tsk_resume_read(void * buf, unsigned int l)
{
	if(complete())
	{
		//handle is idle and open
		len = l;
		set_res_cmd(CMD_READ);
		dst.as_voidptr = buf;
	    tsk_start_handle();
	    if(tsk_resume_wait_signal(signal))
	        res &= ~FLG_SIGNALED;
	    else
	    	tsk_cancel();
	}

    return (res);
}

/**
 * Start write
 * @param buf
 * @param l
 * @return
 */
bool CHandle::tsk_start_write(const void * buf, unsigned int l)
{
	if(!complete())
		return (false);

	//handle is idle and open
	len = l;
	set_res_cmd(CMD_WRITE);
	src.as_voidptr = (void*)buf;
	tsk_start_handle();
	return (true);
}

/**
 * Blocking write
 * @param buf
 * @param l
 * @return
 */
RES_CODE CHandle::tsk_write(const void * buf, unsigned int l)
{
	if(!complete())
		return (res);

	//handle is idle and open
	len = l;
	set_res_cmd(CMD_WRITE);
	src.as_cvoidptr = buf;
	return (tsk_start_and_wait());
}

/**
 * Time limited write
 * @param buf
 * @param l
 * @param time
 * @return
 */
RES_CODE CHandle::tsk_write(const void * buf, unsigned int l, unsigned int time)
{
	if(complete())
	{
		//handle is idle and open
		len = l;
		set_res_cmd(CMD_WRITE);
		src.as_voidptr = (void*)buf;
	    tsk_start_handle();
	    if(tsk_wait_signal(signal, time))
	        res &= ~FLG_SIGNALED;
	    else
	    	tsk_cancel();
	}

    return (res);
}

#if USE_CSTRING || USE_TMOS_TL_V2
RES_CODE CHandle::tsk_read(CSTRING& str, unsigned int size)
{
	char * buf;

	str.clear();
	if(str.reserve(size) >= size)
	{
		buf = (char*)str.c_str();
		res = tsk_read(buf, size);
		buf[size-len] = 0;
		str.m_set_size(strlen(buf));
	} else
		res = RES_OUT_OF_MEMORY;
	return res;
}
#endif

/**
 * Blocking locked write
 * @param buf
 * @param l
 * @return
 */
RES_CODE CHandle::tsk_write_locked(const void * buf, unsigned int l)
{
	if(!complete())
		return (res);

	//handle is idle and open
	len = l;
	set_res_cmd(CMD_WRITE|FLAG_LOCK);
	src.as_voidptr = (void*)buf;
	return (tsk_start_and_wait());
}

/**
 * Time limited locked write
 * @param buf
 * @param l
 * @param time
 * @return
 */
RES_CODE CHandle::tsk_write_locked(const void * buf, unsigned int l, unsigned int time)
{
	if(complete())
	{
		//handle is idle and open
		len = l;
		set_res_cmd(CMD_WRITE|FLAG_LOCK);
		src.as_voidptr = (void*)buf;
	    tsk_start_handle();
	    if(tsk_wait_signal(signal, time))
	        res &= ~FLG_SIGNALED;
	    else
	    	tsk_cancel();
	}

    return (res);
}

/**
 * Resume write operation
 * @param buf
 * @param l
 * @return
 */
RES_CODE CHandle::tsk_resume_write(const void * buf, unsigned int l)
{
	if(complete())
	{
		//handle is idle and open
		len = l;
		set_res_cmd(CMD_WRITE);
		src.as_voidptr = (void*)buf;
	    tsk_start_handle();
	    if(tsk_resume_wait_signal(signal))
	        res &= ~FLG_SIGNALED;
	    else
	    	tsk_cancel();
	}

    return (res);
}

/**
 * Read & write (swap) operation
 * @param d
 * @param s
 * @param l
 * @return
 */
RES_CODE CHandle::tsk_read_write(void *d, const void *s, unsigned int l)
{
	if(!complete())
		return (res);

	//handle is idle and open
	len = l;
	set_res_cmd(CMD_READ_WRITE);
	dst.as_voidptr = d;
	src.as_voidptr = (void*)s;
	return (tsk_start_and_wait());
}

/**
 * Read & write (swap) locked operation
 * @param d
 * @param s
 * @param l
 * @return
 */
RES_CODE CHandle::tsk_read_write_locked(void *d, const void *s, unsigned int l)
{
	if(!complete())
		return (res);

	//handle is idle and open
	len = l;
	set_res_cmd(CMD_READ_WRITE|FLAG_LOCK);
	dst.as_voidptr = d;
	src.as_voidptr = (void*)s;
	return (tsk_start_and_wait());
}

/**
 * Start command
 * @param c
 * @param ptr
 * @return
 */
bool CHandle::tsk_start_command(void * c, void *ptr)
{
	if(!complete())
		return (false);

	//handle is idle and open
	set_res_cmd(CMD_COMMAND);
	dst.as_voidptr = ptr;
	src.as_voidptr = c;
	tsk_start_handle();
	return (true);
}

/**
 * Blocking command
 * @param c
 * @param ptr
 * @return
 */
RES_CODE CHandle::tsk_command(void * c, void *ptr)
{
	if(!complete())
		return (res);

	//handle is idle and open
	set_res_cmd(CMD_COMMAND);
	dst.as_voidptr = ptr;
	src.as_voidptr = c;
	return (tsk_start_and_wait());
}

/**
 *
 * @param cmd
 * @param par
 * @param ptr
 * @return
 */
RES_CODE CHandle::tsk_command(unsigned int c, void * par, void *ptr)
{
	if(!complete())
		return (res);

	//handle is idle and open
	set_res_cmd(c);
	dst.as_voidptr = ptr;
	src.as_voidptr = par;
	return (tsk_start_and_wait());
}

/**
 *
 * @param cmd
 * @param par
 * @param ptr
 * @return
 */
bool CHandle::tsk_start_command(unsigned int c, void * par, void *ptr)
{
	if(!complete())
		return (res);

	//handle is idle and open
	set_res_cmd(c);
	dst.as_voidptr = ptr;
	src.as_voidptr = par;
	tsk_start_handle();
	return (true);
}


/**
 * Locked command
 * @param c
 * @param ptr
 * @return
 */
RES_CODE CHandle::tsk_command_locked(void * c, void *ptr)
{
	if(!complete())
		return (res);

	//handle is idle and open
	set_res_cmd(CMD_COMMAND|FLAG_LOCK);
	dst.as_voidptr = ptr;
	src.as_voidptr = c;
	return (tsk_start_and_wait());
}

/**
 * Driver helper function
 * @param base
 * @return
 */
bool CHandle::svc_list_cancel(HANDLE& base)
{
	if(list_remove(base))
	{
		svc_HND_SET_STATUS(this, RES_SIG_IDLE);
		return (true);
	}
	return (false);
}

/**
 * Driver helper function
 * @param base
 * @return
 */
bool CHandle::list_remove(HANDLE& base)
{
	if(base && base == this )
	{
		base = next;
		return (true);
	}
	HANDLE ptr = base;
	while(ptr && ptr->next)
	{
		if(ptr->next == this )
		{
			ptr->next = next;
			return (true);
		}
		ptr = ptr->next;
	}
	return (false);
}

/**
 * Driver helper function
 * @param base
 */
void CHandle::list_add(HANDLE& base)
{
	HANDLE ptr = base;

	next = 0;
	res = RES_BUSY;
	if(!ptr )
	{
		base = this;
	}
	else
	{
		while(ptr->next)
			ptr=ptr->next;
		ptr->next = this;
	}
}

/**
 * Driver helper function
 */
void CHandle::usr_hnd_signal()
{
    if(client.drv_index > INALID_DRV_INDX)
    {
    	if(client.drv_index & 3)
    	{
    		//the client is a callback
    		client.callback(this, res);
    	} else
    	{
    		//the client is a task
    		usr_send_signal(client.task, signal);
    	}
    } else
    {
        if(client.drv_index < INALID_DRV_INDX)
        	usr_drv_icontrol((DRIVER_INDEX)client.drv_index, signal, this);
    }
}



//*----------------------------------------------------------------------------
//*			Helper functions
//*----------------------------------------------------------------------------
HANDLE tsk_open_new_hnd(DRIVER_INDEX index, const void * mode )
{
	HANDLE hnd;

	if( (hnd = new CHandle()) )
	{
		if(!hnd->tsk_open(index, mode))
		{
			delete hnd;
			hnd = NULL;
		}
	}

	return (hnd);
}

HANDLE tsk_safe_open_new_hnd(DRIVER_INDEX index, const void * mode )
{
	CHandle *hnd;

	while(1)
	{
		if( (hnd = new CHandle()) )
		{
			if(hnd->tsk_open(index, mode))
				break;

			delete hnd;
		}
		tsk_sleep(10);
	}

	return (hnd);
}



