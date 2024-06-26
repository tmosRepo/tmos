#include <tmos.h>
#include <fonts.h>

const char font10x12_iso8859[3360] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 
	0x30, 0xE0, 0x81, 0x07, 0x1E, 0x30, 0xC0, 0x00, 0x03, 0x00, 0x00, 0xC0, 0x00, 0x03, 0xCC, 0x30, 
	0xC3, 0x0C, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xC3, 
	0xCC, 0xFF, 0xFF, 0x33, 0xC3, 0xCC, 0xFF, 0xFF, 0x33, 0xC3, 0x0C, 0x00, 0x30, 0xF0, 0xC3, 0xCF, 
	0x0C, 0x33, 0xF0, 0xC3, 0x0F, 0xCC, 0x30, 0xF3, 0xC3, 0x0F, 0x0C, 0x06, 0x27, 0x9C, 0xB0, 0x31, 
	0xC0, 0xC0, 0x00, 0x03, 0x03, 0x8C, 0x0D, 0x39, 0xE4, 0x60, 0x0C, 0x30, 0x30, 0xC3, 0x0C, 0x0C, 
	0x30, 0x30, 0xF3, 0xCC, 0xC3, 0x0C, 0xC3, 0x33, 0xCF, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x03, 0x03, 0x0C, 0x30, 0xC0, 0x00, 
	0x03, 0x0C, 0x30, 0xC0, 0x00, 0x0C, 0x30, 0x0C, 0x30, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 
	0x0C, 0x30, 0xC0, 0xC0, 0x00, 0x03, 0x00, 0xC0, 0x00, 0xC3, 0xCC, 0x33, 0xF3, 0xC3, 0xCF, 0xCC, 
	0x33, 0xC3, 0x00, 0x03, 0x00, 0x00, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xFC, 0xFF, 0x3F, 0x0C, 0x30, 
	0xC0, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x30, 
	0x30, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xFF, 0xFF, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 
	0x03, 0x00, 0x03, 0x0C, 0x0C, 0x30, 0xC0, 0xC0, 0x00, 0x03, 0x03, 0x0C, 0x30, 0x30, 0xC0, 0x00, 
	0xFC, 0xF8, 0x37, 0xF0, 0xD0, 0x43, 0x8F, 0x3C, 0xF1, 0xC2, 0x0B, 0x0F, 0xEC, 0x1F, 0x3F, 0x30, 
	0xE0, 0xC0, 0x03, 0x0F, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0xC0, 0x0F, 0x3F, 0xFC, 0xF8, 
	0x37, 0xF0, 0xC0, 0x00, 0x03, 0x07, 0x07, 0x07, 0x06, 0x0C, 0xF8, 0xFF, 0xFF, 0xFC, 0xF8, 0x37, 
	0xF0, 0xC0, 0x00, 0xC3, 0x03, 0x0F, 0xC0, 0x03, 0x0F, 0xEC, 0x1F, 0x3F, 0xE0, 0xC0, 0x83, 0x0D, 
	0x33, 0xC6, 0x0C, 0xF3, 0xFF, 0xFF, 0xC0, 0x00, 0x03, 0x0C, 0x78, 0xFF, 0xFF, 0x3F, 0xC0, 0x00, 
	0xFF, 0xFC, 0x07, 0x30, 0xC0, 0x03, 0x0F, 0xEC, 0x1F, 0x3F, 0xFC, 0xF8, 0x37, 0xF0, 0x00, 0x03, 
	0xFC, 0xF7, 0xFF, 0xC0, 0x03, 0x0F, 0xEC, 0x1F, 0x3F, 0xFF, 0xFF, 0x3F, 0x30, 0xC0, 0x80, 0x01, 
	0x03, 0x06, 0x18, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0xFC, 0xF8, 0x37, 0xF0, 0xC0, 0x02, 0xF1, 0xE3, 
	0xDF, 0xC0, 0x03, 0x0F, 0xEC, 0x1F, 0x3F, 0xFC, 0xF8, 0x37, 0xF0, 0xC0, 0x03, 0xFB, 0xCF, 0x3F, 
	0xC0, 0x00, 0x0F, 0xEC, 0x1F, 0x3F, 0x00, 0xC0, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xC0, 0x00, 0x03, 0x00, 0x00, 0x30, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 
	0x30, 0x30, 0xC0, 0x00, 0x00, 0x00, 0x03, 0x0C, 0x0C, 0x30, 0x30, 0xC0, 0x00, 0x0C, 0x30, 0x00, 
	0x03, 0x0C, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0xFF, 0x03, 0x00, 0xC0, 0xFF, 0xFF, 0x03, 0x00, 
	0x00, 0x00, 0x00, 0x30, 0xC0, 0x00, 0x0C, 0x30, 0x00, 0x03, 0x0C, 0x0C, 0x30, 0x30, 0xC0, 0x00, 
	0x00, 0xFC, 0xF8, 0x37, 0x70, 0xC0, 0x00, 0x03, 0x07, 0x0E, 0x0C, 0x30, 0x00, 0x00, 0x03, 0x0C, 
	0xFC, 0xF8, 0x37, 0xF0, 0xC0, 0x00, 0x63, 0xEC, 0xF3, 0xCC, 0x33, 0xCF, 0xE4, 0x0F, 0x1F, 0x30, 
	0xE0, 0xC1, 0x8C, 0x61, 0x03, 0x0F, 0xFC, 0xFF, 0xFF, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0xFF, 0xFC, 
	0x67, 0xB0, 0xC1, 0x06, 0xFB, 0xE3, 0x8F, 0xC1, 0x06, 0x1B, 0xFC, 0xDF, 0x3F, 0xFC, 0xF8, 0x37, 
	0xF0, 0xC0, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0F, 0xEC, 0x1F, 0x3F, 0xFF, 0xFC, 0x67, 0xB0, 
	0xC1, 0x06, 0x1B, 0x6C, 0xB0, 0xC1, 0x06, 0x1B, 0xF4, 0xDF, 0x3F, 0xFF, 0xFF, 0x6F, 0xA0, 0x01, 
	0x06, 0xF9, 0xE7, 0x9F, 0x41, 0x06, 0x18, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0x6F, 0xA0, 0x01, 0x06, 
	0xF9, 0xE7, 0x9F, 0x41, 0x06, 0x18, 0x60, 0xC0, 0x03, 0xFC, 0xF8, 0x37, 0xF0, 0xC0, 0x03, 0x0C, 
	0x30, 0xFE, 0xF8, 0x03, 0x0F, 0xEC, 0x1F, 0x3F, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0xFF, 0xFF, 
	0xFF, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0xFC, 0xF0, 0x03, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 
	0x0C, 0x30, 0xC0, 0xC0, 0x0F, 0x3F, 0xC0, 0x03, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 
	0x80, 0x31, 0x86, 0x1F, 0x3C, 0xCF, 0x1B, 0x66, 0x8C, 0x19, 0x36, 0x78, 0xE0, 0x81, 0x0D, 0x66, 
	0x18, 0x63, 0xD8, 0xF3, 0x0F, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 
	0xF8, 0xFF, 0xFF, 0x03, 0x0F, 0x7C, 0xF8, 0xF3, 0x7B, 0xCF, 0x3C, 0xF3, 0xC0, 0x03, 0x0F, 0x3C, 
	0xF0, 0xC0, 0x03, 0x0F, 0x7C, 0xF0, 0xC3, 0x1B, 0xCF, 0x3C, 0xF6, 0xF0, 0x83, 0x0F, 0x3C, 0xF0, 
	0xC0, 0xFC, 0xF8, 0x37, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0xEC, 0x1F, 0x3F, 
	0x7F, 0xFC, 0x63, 0x98, 0x61, 0x86, 0xF9, 0xE3, 0x87, 0x01, 0x06, 0x18, 0x60, 0xC0, 0x03, 0xFC, 
	0xF8, 0x37, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xDC, 0xC3, 0x0D, 0xE3, 0x37, 0x8F, 0xFF, 0xFC, 
	0x67, 0x98, 0x61, 0x86, 0xF9, 0xE7, 0x8F, 0x0D, 0x66, 0x18, 0x63, 0xD8, 0xE3, 0xFC, 0xF8, 0x37, 
	0xF0, 0xC0, 0x03, 0xF8, 0xC3, 0x1F, 0xC0, 0x03, 0x0F, 0xEC, 0x1F, 0x3F, 0xFF, 0xFF, 0x1F, 0x23, 
	0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x1E, 0xCF, 0x1B, 0x66, 0x98, 0x61, 
	0x86, 0x19, 0x66, 0x98, 0x61, 0x86, 0x19, 0xC6, 0x0F, 0x1E, 0xCF, 0x1B, 0x66, 0x98, 0x61, 0x86, 
	0x19, 0xC6, 0x0C, 0x33, 0xCC, 0x30, 0x83, 0x07, 0x0C, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 
	0x3C, 0xF3, 0xCC, 0xCE, 0x39, 0xC7, 0x0C, 0x33, 0x03, 0x0F, 0x3C, 0x30, 0x33, 0xCC, 0xC0, 0x00, 
	0x03, 0x33, 0xCC, 0x0C, 0x3C, 0xF0, 0xC0, 0xCF, 0x1B, 0x66, 0x98, 0x61, 0xCE, 0xF1, 0x83, 0x07, 
	0x0C, 0x30, 0xC0, 0x00, 0x03, 0x1E, 0xFF, 0xFF, 0x1F, 0x30, 0x60, 0xC0, 0x80, 0x81, 0x03, 0x03, 
	0x06, 0x0C, 0xF8, 0xFF, 0xFF, 0xF0, 0xC0, 0x03, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 
	0xC0, 0x00, 0x0F, 0x3C, 0x03, 0x0C, 0xC0, 0x00, 0x03, 0x0C, 0xC0, 0x00, 0x03, 0x30, 0xC0, 0x00, 
	0x03, 0x30, 0xC0, 0x3C, 0xF0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0xC0, 
	0x03, 0x0F, 0x30, 0xC0, 0xC0, 0x0C, 0x33, 0x03, 0x0F, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xFF, 0xFF, 
	0x30, 0xC0, 0x00, 0x0C, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xC0, 0x8F, 0x7F, 0x00, 0x03, 0xCC, 0xBF, 0xFF, 0x03, 0x0F, 0xEC, 0x3F, 0xDF, 0x03, 0x0C, 
	0x30, 0xC0, 0x00, 0xF3, 0xCC, 0xF3, 0xF0, 0xC3, 0x03, 0x0F, 0xFC, 0xCF, 0x3F, 0x00, 0x00, 0xC0, 
	0x8F, 0x7F, 0x07, 0x0F, 0x38, 0xC0, 0x00, 0x03, 0x1E, 0xEC, 0x1F, 0x3F, 0x00, 0x03, 0x0C, 0x30, 
	0xC0, 0x3C, 0xF3, 0x3C, 0xFC, 0xF0, 0x03, 0x0F, 0xCC, 0x3F, 0xFF, 0x00, 0x00, 0xC0, 0x0F, 0x3F, 
	0x03, 0x0F, 0xFC, 0xFF, 0xFF, 0x03, 0x0C, 0xC0, 0x0F, 0x3F, 0xF0, 0xC0, 0xC3, 0x30, 0xC3, 0x0C, 
	0x30, 0xF0, 0xCF, 0x3F, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x00, 0x00, 0xC0, 0x3F, 0xFF, 0x03, 0x0C, 
	0x30, 0xFF, 0xFC, 0x03, 0x0F, 0xCC, 0x0F, 0x3F, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0xCC, 0x33, 
	0xCF, 0xC3, 0x0F, 0x0F, 0x3C, 0xF0, 0xC0, 0x30, 0xC0, 0x00, 0x00, 0x00, 0x3C, 0xF0, 0x00, 0x03, 
	0x0C, 0x30, 0xC0, 0xF0, 0xFF, 0xFF, 0x00, 0x00, 0x03, 0x0C, 0x00, 0xFC, 0xF3, 0x0F, 0x0C, 0x30, 
	0xC3, 0x0C, 0xC3, 0x03, 0x0F, 0x03, 0x0C, 0x30, 0xCC, 0x30, 0x33, 0xCC, 0xF0, 0xC0, 0x03, 0x33, 
	0xCC, 0x30, 0xFC, 0xF0, 0x3C, 0xF0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 
	0xF0, 0xFF, 0xFF, 0x00, 0x00, 0xF0, 0xFC, 0xF3, 0x33, 0xCF, 0x3C, 0xF3, 0xCC, 0x03, 0x0F, 0x3C, 
	0xF0, 0xC0, 0x00, 0x00, 0x30, 0xCF, 0x3C, 0x0F, 0x3F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 
	0xC0, 0x00, 0x00, 0xC0, 0x0F, 0x3F, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0xCC, 0x0F, 0x3F, 
	0x00, 0x00, 0xF0, 0xCF, 0x3F, 0x03, 0x0F, 0xFC, 0xCF, 0x3F, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x00, 
	0x00, 0xC0, 0x3F, 0xFF, 0x03, 0x0F, 0x3C, 0xFC, 0xF0, 0x3C, 0xF3, 0x0C, 0x30, 0xC0, 0x00, 0x00, 
	0x30, 0xFF, 0xFC, 0x0F, 0x3C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x00, 0x00, 0xC0, 
	0x3F, 0xFF, 0x03, 0x0C, 0xC0, 0x0F, 0x3F, 0x00, 0x03, 0xFC, 0xCF, 0x3F, 0x0C, 0x30, 0xC0, 0x00, 
	0x03, 0x3F, 0xFC, 0xC0, 0x00, 0x03, 0x0C, 0x33, 0x0C, 0x0F, 0x3C, 0x00, 0x00, 0x30, 0xF0, 0xC0, 
	0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0xCC, 0x3F, 0xFF, 0x00, 0x00, 0x30, 0xF0, 0xC0, 0x03, 
	0x0F, 0x3C, 0xF0, 0xC0, 0xCC, 0x30, 0x03, 0x03, 0x0C, 0x00, 0x00, 0x30, 0xF0, 0xC0, 0x33, 0xCF, 
	0x3C, 0xF3, 0xCC, 0x33, 0xCF, 0xFC, 0xFC, 0xF3, 0x00, 0x00, 0x30, 0xF0, 0xC0, 0xCC, 0x30, 0x03, 
	0x03, 0x0C, 0xCC, 0x30, 0x33, 0xF0, 0xC0, 0x00, 0x00, 0x30, 0xF0, 0xC0, 0x03, 0x0F, 0xCC, 0x3F, 
	0xFF, 0x00, 0x03, 0xCC, 0x0F, 0x3F, 0x00, 0x00, 0xF0, 0xFF, 0xFF, 0x80, 0x01, 0x03, 0x06, 0x0E, 
	0x0C, 0x18, 0xF0, 0xFF, 0xFF, 0xF0, 0xC0, 0x03, 0x03, 0x0C, 0x30, 0xF0, 0xC0, 0x03, 0x0C, 0x30, 
	0xC0, 0x00, 0x0F, 0x3C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 
	0x00, 0x03, 0x0C, 0x3C, 0xF0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x03, 0x0F, 0x0C, 0x30, 0xC0, 0xC0, 
	0x03, 0x0F, 0x00, 0x00, 0xC0, 0x00, 0x03, 0x33, 0xCF, 0x0C, 0x0C, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xFC, 0xF0, 0xC3, 0x0C, 0x33, 0xCC, 0x30, 0xC3, 0x0C, 0x33, 0xCC, 0x30, 0xC3, 0x0F, 0x3F, 
	0x00, 0xE0, 0xC3, 0x9F, 0x01, 0x06, 0x7C, 0x60, 0xC0, 0x07, 0x06, 0x18, 0xC0, 0x1F, 0x3E, 0xC0, 
	0x00, 0xF3, 0xFF, 0xFF, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x38, 0xC0, 0x80, 0x01, 0x03, 0xE0, 0xC0, 0x07, 
	0x03, 0x0C, 0xFC, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC8, 0xE0, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCC, 0x30, 0x83, 0x08, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0xCF, 0x0C, 0x00, 0x00, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0xFF, 
	0xFF, 0x0F, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0xF0, 0xFF, 0xFF, 0x30, 0xC0, 
	0x00, 0x03, 0x0C, 0xFF, 0xFF, 0x0F, 0x03, 0x0C, 0x30, 0xE0, 0xC1, 0x8C, 0x61, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x27, 0x96, 0x8C, 0x19, 0x30, 0x60, 0xC0, 0x80, 
	0x01, 0x8F, 0x4D, 0x29, 0x25, 0x63, 0x48, 0xC0, 0xC0, 0x8F, 0x7F, 0x03, 0x0E, 0xE0, 0x0F, 0x7F, 
	0x00, 0x07, 0xEC, 0x1F, 0x3F, 0x00, 0x00, 0x00, 0x0C, 0x38, 0x38, 0x30, 0xC0, 0x00, 0x0E, 0xE0, 
	0x00, 0x03, 0x00, 0x00, 0xFC, 0xFB, 0x7F, 0xC6, 0x18, 0x63, 0x8C, 0x3F, 0xFE, 0x18, 0x63, 0x9C, 
	0xE1, 0x3F, 0xFF, 0x13, 0x2F, 0x36, 0xCC, 0x18, 0x33, 0x6C, 0xF0, 0xC1, 0x0D, 0x63, 0x0C, 0x33, 
	0xD8, 0xC0, 0x48, 0xC0, 0xF0, 0xFF, 0xFF, 0x00, 0x03, 0x06, 0x1E, 0x1E, 0x1E, 0x1C, 0xF0, 0xFF, 
	0xFF, 0x86, 0x19, 0x66, 0x98, 0x61, 0x86, 0x19, 0x66, 0x98, 0x61, 0xFE, 0xF9, 0x07, 0x03, 0x0C, 
	0x0C, 0x30, 0xF0, 0xC3, 0x0F, 0x0C, 0x30, 0xC7, 0x3E, 0xCF, 0x1C, 0x33, 0xCC, 0x18, 0x33, 0x20, 
	0x40, 0x00, 0x07, 0x1C, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xE0, 
	0x80, 0x03, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x08, 0xE1, 
	0x9C, 0x73, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x38, 0xE7, 0x1C, 
	0x42, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x03, 0x3F, 
	0xFC, 0xFC, 0xFF, 0x3F, 0x3F, 0xFC, 0xC0, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 
	0xE0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xE2, 0xCC, 0x1E, 0x31, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2F, 0xBE, 0x6D, 0xBE, 0xF9, 0x66, 0x9B, 0x0D, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0xC0, 0xC0, 0x9F, 0xFF, 0x03, 0x0C, 0xE0, 0x0F, 0x3F, 
	0x00, 0x03, 0xFC, 0x9F, 0x3F, 0x00, 0x00, 0xC0, 0x00, 0x07, 0x70, 0x00, 0x03, 0x0C, 0x1C, 0x1C, 
	0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xDC, 0xFF, 0x33, 0xCE, 0x38, 0xFF, 0x7C, 0x33, 0xCC, 
	0xF0, 0xBF, 0x73, 0x00, 0x40, 0xB0, 0xCC, 0x30, 0x33, 0xCC, 0xF0, 0xC0, 0x03, 0x33, 0xCC, 0x30, 
	0xCC, 0x30, 0x48, 0xC0, 0xF0, 0xFF, 0xFF, 0x80, 0x01, 0x03, 0x06, 0x0E, 0x0C, 0x18, 0xF0, 0xFF, 
	0xFF, 0x4B, 0x2F, 0x3D, 0xF0, 0xE1, 0xCE, 0xF1, 0x83, 0x07, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 
	0x40, 0x00, 0x07, 0x1C, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xE0, 
	0x80, 0x03, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x81, 
	0x09, 0x26, 0x18, 0xF0, 0xC1, 0x07, 0x06, 0x18, 0x60, 0xE0, 0x8F, 0x3F, 0xF8, 0xF0, 0x67, 0x80, 
	0x01, 0x1F, 0x18, 0xF0, 0x81, 0x01, 0x06, 0xF0, 0x87, 0x0F, 0x00, 0xFC, 0xF8, 0x37, 0xE0, 0x00, 
	0x33, 0xCF, 0x3E, 0xCF, 0x1C, 0x33, 0xCC, 0xF8, 0x9F, 0x3F, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 
	0x00, 0x00, 0x00, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0xFC, 0xF8, 0x37, 0xE0, 0x00, 0x7E, 0x30, 
	0xC3, 0x0C, 0x3E, 0x00, 0x07, 0xFC, 0xBF, 0x7F, 0xCC, 0x30, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xF8, 0x37, 0xF0, 0xCC, 0x0B, 0x2F, 0x3C, 0xF3, 
	0xC0, 0xFE, 0xF1, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x18, 0x60, 0x80, 0x01, 0x0E, 0x00, 0x80, 0x08, 0x33, 0x66, 0xCC, 0x98, 0x61, 0x06, 0x33, 0x98, 
	0xC1, 0x0C, 0x22, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x3F, 0xFF, 0x00, 0x03, 0x0C, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xFF, 0xFC, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x30, 0xE0, 0xC1, 0x0C, 0x33, 0x78, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0x0C, 0xFC, 0xF0, 0x03, 0x03, 0x0C, 0x00, 0xF0, 0xC3, 0x0F, 0x00, 0x1C, 0xD8, 
	0x00, 0x03, 0x06, 0x0C, 0xF8, 0xE0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0xC8, 0x00, 
	0x02, 0x07, 0x20, 0xC8, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x60, 0xC0, 0x80, 
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0xCC, 0xBC, 0x31, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0xE4, 0xC1, 0x8C, 0x61, 0x03, 
	0x0F, 0xFC, 0xFF, 0xFF, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xC0, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD, 0xF7, 0x9F, 0x21, 0x06, 0x18, 0xE1, 0x87, 
	0x1F, 0x46, 0x18, 0x60, 0xC8, 0x3F, 0xFF, 0x0D, 0x37, 0xDC, 0x30, 0xC3, 0x0C, 0xF3, 0xCF, 0x3F, 
	0xC3, 0x0C, 0x33, 0xCC, 0x30, 0xC3, 0x79, 0xE4, 0x11, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 
	0x30, 0xC0, 0x80, 0x07, 0x1E, 0x00, 0x44, 0x30, 0x83, 0x19, 0xCC, 0x60, 0x86, 0x19, 0x33, 0x66, 
	0xCC, 0x10, 0x01, 0x00, 0xF1, 0xE4, 0xD7, 0x30, 0xC3, 0x0C, 0x33, 0xCC, 0x30, 0xC3, 0x0C, 0x33, 
	0x8C, 0x1F, 0x3C, 0x04, 0x18, 0x70, 0x80, 0x21, 0x46, 0xBC, 0x00, 0x1D, 0xCA, 0x04, 0x0B, 0x06, 
	0x0C, 0xF8, 0x0D, 0x37, 0xDC, 0x30, 0xC3, 0x9C, 0xE3, 0x07, 0x0F, 0x18, 0x60, 0x80, 0x01, 0x06, 
	0x3C, 0xF1, 0xE4, 0xD7, 0x30, 0xC3, 0x0C, 0x33, 0xCC, 0x30, 0xC3, 0x98, 0x41, 0xC2, 0x39, 0xE7, 
	0x30, 0x58, 0x66, 0x18, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x1E, 0x30, 
	0xE0, 0xC1, 0x8C, 0x61, 0x03, 0x0F, 0x3C, 0xF0, 0xFF, 0xFF, 0x0F, 0x3C, 0xF0, 0xC0, 0xFF, 0xFC, 
	0x67, 0xB0, 0xC1, 0x06, 0xFB, 0xE3, 0x8F, 0xC1, 0x06, 0x1B, 0xFC, 0xDF, 0x3F, 0xFF, 0xFF, 0x6F, 
	0xA0, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0xC0, 0x03, 0x30, 0xC0, 0x80, 0x07, 
	0x1E, 0xCC, 0x10, 0x62, 0x98, 0x40, 0x03, 0x0F, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x6F, 0xA0, 0x01, 
	0x06, 0xF9, 0xE7, 0x9F, 0x41, 0x06, 0x18, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0x30, 0x60, 0xC0, 
	0x80, 0x01, 0x03, 0x06, 0x0C, 0x18, 0xF0, 0xFF, 0xFF, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0xFF, 
	0xFF, 0xFF, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0xFC, 0xF8, 0x37, 0xF0, 0xC0, 0x03, 0xEF, 0xBD, 
	0xF7, 0xC0, 0x03, 0x0F, 0xEC, 0x1F, 0x3F, 0x78, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 
	0x0C, 0x30, 0xC0, 0x00, 0x03, 0x1E, 0xCF, 0x1B, 0x66, 0x8C, 0x19, 0x36, 0x78, 0xE0, 0x81, 0x0D, 
	0x66, 0x18, 0x63, 0xD8, 0xF3, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0xCC, 0x30, 0xC3, 0x0C, 0x33, 0x86, 
	0x19, 0x66, 0xD8, 0xF3, 0x03, 0x0F, 0x7C, 0xF8, 0xF3, 0x7B, 0xCF, 0x3C, 0xF3, 0xC0, 0x03, 0x0F, 
	0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x7C, 0xF0, 0xC3, 0x1B, 0xCF, 0x3C, 0xF6, 0xF0, 0x83, 0x0F, 0x3C, 
	0xF0, 0xC0, 0xFF, 0xFF, 0x3F, 0xF0, 0xC0, 0x00, 0xE0, 0x81, 0x07, 0x00, 0x03, 0x0F, 0xFC, 0xFF, 
	0xFF, 0xFC, 0xF8, 0x37, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0xEC, 0x1F, 0x3F, 
	0xFF, 0xFF, 0x6F, 0x98, 0x61, 0x86, 0x19, 0x66, 0x98, 0x61, 0x86, 0x19, 0x66, 0x98, 0x61, 0x7F, 
	0xFC, 0x63, 0x98, 0x61, 0x86, 0xF9, 0xE3, 0x87, 0x01, 0x06, 0x18, 0x60, 0xC0, 0x03, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x6F, 
	0x30, 0x03, 0x18, 0xC0, 0x00, 0x03, 0x06, 0x0C, 0x18, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0x23, 
	0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x1E, 0xCF, 0x1B, 0x66, 0x18, 0x33, 
	0xCC, 0xE0, 0x81, 0x07, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x1E, 0x78, 0xC0, 0x00, 0x83, 0x7F, 0x33, 
	0xC7, 0x18, 0xE3, 0xCC, 0xFE, 0xC1, 0x00, 0x03, 0x3E, 0x01, 0x0E, 0x6C, 0x18, 0x21, 0xCC, 0xC0, 
	0x00, 0x03, 0x33, 0x84, 0x18, 0x36, 0x70, 0x80, 0x03, 0xCF, 0x3C, 0xF3, 0xCC, 0x33, 0xCF, 0x7C, 
	0xBB, 0x7F, 0xFC, 0xC0, 0x00, 0x03, 0x1E, 0x78, 0xF8, 0x77, 0xF8, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 
	0xC0, 0x86, 0x31, 0xF3, 0xFC, 0xF3, 0xCC, 0x30, 0x03, 0x00, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 
	0x30, 0xC0, 0x00, 0x03, 0x0C, 0xCC, 0x30, 0x13, 0xE0, 0xC0, 0x86, 0x31, 0x83, 0x07, 0x0C, 0x30, 
	0xC0, 0x00, 0x03, 0x0C, 0x30, 0x00, 0x00, 0x00, 0x8F, 0x7E, 0x0F, 0x37, 0xD8, 0x60, 0x83, 0x0D, 
	0xE7, 0x37, 0x8F, 0x30, 0x00, 0xE0, 0xDF, 0xFF, 0x03, 0x0E, 0xE0, 0x87, 0x1F, 0x03, 0x0C, 0xF8, 
	0xBF, 0x7F, 0x30, 0x00, 0x60, 0x80, 0x39, 0xF6, 0x39, 0x6C, 0xB0, 0xC1, 0x06, 0x1B, 0x6C, 0x30, 
	0xC0, 0x30, 0x40, 0x00, 0x00, 0x00, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x18, 
	0x60, 0x98, 0x6C, 0x30, 0x00, 0x86, 0x18, 0x64, 0xB0, 0xC1, 0x06, 0x1B, 0xCC, 0x1F, 0x3E, 0x00, 
	0x00, 0x00, 0x00, 0x8F, 0x7E, 0x0F, 0x37, 0xD8, 0x60, 0x83, 0x0D, 0xE7, 0x37, 0x8F, 0x00, 0x00, 
	0x80, 0x0F, 0x77, 0x06, 0x19, 0x62, 0x8F, 0x3D, 0x86, 0x39, 0xA4, 0xDD, 0x1C, 0x00, 0x00, 0x00, 
	0x80, 0x31, 0x86, 0x31, 0x84, 0x19, 0x3C, 0x70, 0xC0, 0x00, 0x03, 0x06, 0x00, 0x00, 0x80, 0x1F, 
	0x7F, 0x0C, 0x60, 0x80, 0x1F, 0xFF, 0x0E, 0x1B, 0xCC, 0x3F, 0x7E, 0x00, 0x00, 0xE0, 0xDF, 0xFF, 
	0x03, 0x0E, 0xE0, 0x87, 0x1F, 0x03, 0x0C, 0xF8, 0xBF, 0x7F, 0x00, 0xF0, 0xCF, 0x3F, 0xC0, 0xC0, 
	0xC1, 0xC1, 0x81, 0x01, 0x06, 0x78, 0x80, 0x3F, 0xC0, 0x00, 0x00, 0x30, 0xC0, 0x3C, 0xFB, 0x1D, 
	0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0x30, 0xC0, 0x00, 0x00, 0x80, 0x07, 0x3F, 0x86, 0x19, 0xE6, 
	0x9F, 0x7F, 0x86, 0x19, 0xC6, 0x0F, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x30, 0xC0, 0x00, 0x03, 
	0x0C, 0x30, 0xC0, 0x00, 0x03, 0x18, 0x00, 0x00, 0x00, 0x80, 0xC3, 0x8C, 0x31, 0xC3, 0x06, 0x0F, 
	0x6C, 0x30, 0xC3, 0x98, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x60, 0x80, 0x01, 0x0C, 0x38, 0xB0, 
	0x61, 0xC6, 0xB0, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86, 0x19, 0x66, 0x98, 0x73, 0xFE, 0xD9, 
	0x66, 0x98, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x1B, 0xCC, 0x18, 0x63, 0x88, 0x60, 0x03, 
	0x07, 0x08, 0x00, 0xF8, 0xEF, 0x3F, 0x18, 0x30, 0xF0, 0xC7, 0x9F, 0x01, 0x06, 0xF8, 0xCF, 0x3F, 
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xF0, 0x63, 0x98, 0x61, 0x86, 0x19, 0xC6, 0x0F, 0x1E, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFB, 0xCF, 0x18, 0x63, 0x8C, 0x31, 0xC6, 0x18, 0x63, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x7C, 0xF8, 0x63, 0x98, 0x61, 0xFE, 0xD9, 0x63, 0x80, 0x01, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xFC, 0xF9, 0x6F, 0x80, 0x01, 0x06, 0xF8, 0xC7, 0x1F, 0x60, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xF8, 0xF3, 0x6F, 0x8C, 0x61, 0x86, 0x19, 0xC6, 0x0F, 0x1E, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xFE, 0xF9, 0x07, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xC6, 0x18, 0x66, 0x98, 0x61, 0x86, 0x19, 0xE7, 0x0F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE2, 
	0xCD, 0x3F, 0xF3, 0xCC, 0x33, 0xFB, 0xC7, 0x0F, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x3A, 
	0xAC, 0x19, 0x34, 0x30, 0xE0, 0xC1, 0xAC, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0F, 0x3C, 
	0xF3, 0xCC, 0x33, 0xFF, 0xEF, 0x1F, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86, 0x0D, 0x3C, 0xF0, 
	0xCC, 0x33, 0xCF, 0xEC, 0x1F, 0x33, 0xCC, 0x30, 0x03, 0x00, 0x00, 0x30, 0xC0, 0x00, 0x03, 0x0C, 
	0x30, 0xC0, 0x00, 0x03, 0x18, 0xCC, 0x30, 0x03, 0x00, 0x00, 0xC6, 0x18, 0x66, 0x98, 0x61, 0x86, 
	0x19, 0xE7, 0x0F, 0x1F, 0x60, 0xC0, 0x00, 0x00, 0x00, 0x78, 0xF0, 0x63, 0x98, 0x61, 0x86, 0x19, 
	0xC6, 0x0F, 0x1E, 0x60, 0xC0, 0x00, 0x00, 0x00, 0xC6, 0x18, 0x66, 0x98, 0x61, 0x86, 0x19, 0xE7, 
	0x0F, 0x1F, 0x60, 0xC0, 0x00, 0x00, 0x00, 0x86, 0x0D, 0x3C, 0xF0, 0xCC, 0x33, 0xCF, 0xEC, 0x1F, 
	0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

