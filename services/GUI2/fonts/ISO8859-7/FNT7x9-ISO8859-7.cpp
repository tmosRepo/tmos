#include <tmos.h>
#include <fonts.h>

const char font7x9_iso8859[1792] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x06, 0x83, 0x01, 0x00, 0x30, 0x18, 0x00, 
	0x12, 0x89, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x91, 0xE8, 0x2F, 0xFA, 0x8B, 0x44, 0x00, 
	0x00, 0x84, 0x2A, 0xE1, 0x43, 0xAA, 0x10, 0x00, 0x80, 0xE3, 0x09, 0x82, 0x20, 0xC8, 0xE3, 0x00, 
	0x00, 0x8E, 0x88, 0xA2, 0x88, 0x86, 0x9C, 0x00, 0x18, 0x0C, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x10, 0x04, 0x81, 0x40, 0x20, 0x10, 0x10, 0x10, 0x04, 0x04, 0x04, 0x02, 0x81, 0x40, 0x10, 0x04, 
	0x00, 0x44, 0x52, 0xC5, 0x51, 0x25, 0x11, 0x00, 0x00, 0x00, 0x02, 0xF1, 0x47, 0x20, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xE0, 0x70, 0x20, 0x08, 0x00, 0x00, 0x00, 0xF0, 0x07, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xE0, 0x70, 0x00, 0x00, 0x00, 0x20, 0x08, 0x82, 0x20, 0x08, 0x02, 0x00, 
	0x00, 0x8E, 0x28, 0x18, 0x0C, 0x8A, 0x38, 0x00, 0x00, 0x84, 0x02, 0x81, 0x40, 0x20, 0x7C, 0x00, 
	0x00, 0x55, 0x10, 0x08, 0x21, 0x04, 0xFE, 0x00, 0x00, 0x5F, 0x10, 0x88, 0x03, 0x06, 0x7D, 0x00, 
	0x00, 0x08, 0x86, 0x22, 0xF1, 0x41, 0x20, 0x00, 0x80, 0x7F, 0xE0, 0x07, 0x04, 0x06, 0x7D, 0x00, 
	0x00, 0x5F, 0x20, 0xF0, 0x0B, 0x06, 0x7D, 0x00, 0x80, 0x7F, 0x10, 0x04, 0x81, 0x20, 0x10, 0x00, 
	0x00, 0x5F, 0x30, 0xE8, 0x0B, 0x06, 0x7D, 0x00, 0x00, 0x5F, 0x30, 0xE8, 0x07, 0x02, 0x7D, 0x00, 
	0x00, 0x00, 0x83, 0x01, 0x60, 0x30, 0x00, 0x00, 0x00, 0x00, 0x83, 0x01, 0x60, 0x30, 0x10, 0x04, 
	0x40, 0x08, 0x21, 0x40, 0x80, 0x00, 0x01, 0x00, 0x00, 0xC0, 0x1F, 0xF0, 0x07, 0x00, 0x00, 0x00, 
	0x01, 0x02, 0x04, 0x08, 0x21, 0x04, 0x00, 0x00, 0xBE, 0x20, 0x10, 0x82, 0x00, 0x20, 0x00, 0x00, 
	0x00, 0x5F, 0x30, 0x9A, 0x8C, 0x07, 0x7C, 0x00, 0x00, 0x8E, 0x28, 0x18, 0xFC, 0x07, 0x83, 0x00, 
	0x80, 0x9F, 0x50, 0xE8, 0x13, 0x0A, 0x7F, 0x00, 0x00, 0x5F, 0x30, 0x10, 0x08, 0x04, 0x7D, 0x00, 
	0x80, 0x9F, 0x50, 0x28, 0x14, 0x0A, 0x7F, 0x00, 0x80, 0xBF, 0x40, 0xE0, 0x13, 0x08, 0xFE, 0x00, 
	0x80, 0x9F, 0x40, 0xE0, 0x13, 0x08, 0x0E, 0x00, 0x00, 0x5F, 0x30, 0x10, 0x0F, 0x06, 0x7D, 0x00, 
	0x80, 0x60, 0x30, 0xF8, 0x0F, 0x06, 0x83, 0x00, 0x00, 0x1F, 0x02, 0x81, 0x40, 0x20, 0x7C, 0x00, 
	0x00, 0x1E, 0x04, 0x02, 0x81, 0x48, 0x18, 0x00, 0x80, 0xB3, 0x44, 0x61, 0x90, 0x88, 0xCE, 0x00, 
	0x80, 0x83, 0x40, 0x20, 0x10, 0x08, 0xFF, 0x00, 0x80, 0xE0, 0xB8, 0x9A, 0x0C, 0x06, 0x83, 0x00, 
	0x80, 0xE0, 0xB0, 0x98, 0x8C, 0x86, 0x83, 0x00, 0x00, 0x5F, 0x30, 0x18, 0x0C, 0x06, 0x7D, 0x00, 
	0x80, 0x9F, 0x50, 0xE8, 0x13, 0x08, 0x0E, 0x00, 0x00, 0x5F, 0x30, 0x18, 0x8C, 0x86, 0xBC, 0x00, 
	0x80, 0x8F, 0x48, 0xE4, 0x91, 0x88, 0xCE, 0x00, 0x00, 0x5F, 0x30, 0xE0, 0x03, 0x06, 0x7D, 0x00, 
	0x80, 0x7F, 0x12, 0x81, 0x40, 0x20, 0x38, 0x00, 0x80, 0xBB, 0x48, 0x24, 0x12, 0x89, 0x38, 0x00, 
	0x80, 0x60, 0x50, 0x24, 0xA2, 0x50, 0x10, 0x00, 0x80, 0x60, 0x30, 0x99, 0x4C, 0x56, 0x45, 0x00, 
	0x80, 0xA0, 0x88, 0x82, 0xA0, 0x88, 0x82, 0x00, 0x80, 0xA0, 0x88, 0x82, 0x40, 0x20, 0x38, 0x00, 
	0x80, 0x3F, 0x08, 0x82, 0x20, 0x08, 0xFE, 0x00, 0x7C, 0x02, 0x81, 0x40, 0x20, 0x10, 0x08, 0x7C, 
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x1F, 0x08, 0x04, 0x02, 0x81, 0x40, 0x20, 0x1F, 
	0x08, 0x51, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 
	0x06, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x07, 0xF4, 0x07, 0xFD, 0x00, 
	0x80, 0x40, 0xE0, 0x17, 0x0C, 0x06, 0x7F, 0x00, 0x00, 0x00, 0xC0, 0x1F, 0x08, 0x04, 0xFC, 0x00, 
	0x00, 0x20, 0xD0, 0x1F, 0x0C, 0x06, 0xFD, 0x00, 0x00, 0x00, 0xC0, 0x17, 0xFC, 0x07, 0x7C, 0x00, 
	0x00, 0x3C, 0xE1, 0x4F, 0x20, 0x10, 0x08, 0x00, 0x00, 0x80, 0x2F, 0x18, 0x0C, 0xFA, 0x81, 0x3E, 
	0x80, 0x40, 0xA0, 0x37, 0x0C, 0x06, 0x83, 0x00, 0x00, 0x04, 0xC0, 0x81, 0x40, 0x20, 0x7C, 0x00, 
	0x10, 0x00, 0x07, 0x02, 0x81, 0x40, 0x20, 0x0E, 0x80, 0x40, 0x20, 0x18, 0xB9, 0x84, 0x82, 0x00, 
	0x00, 0x07, 0x02, 0x81, 0x40, 0x20, 0x7C, 0x00, 0x00, 0x00, 0xA0, 0x94, 0x4C, 0x26, 0x93, 0x00, 
	0x00, 0x00, 0x20, 0x55, 0x0C, 0x06, 0x83, 0x00, 0x00, 0x00, 0xC0, 0x17, 0x0C, 0x06, 0x7D, 0x00, 
	0x00, 0x00, 0xE0, 0x13, 0x0C, 0x7E, 0x02, 0x01, 0x00, 0x00, 0xC0, 0x1F, 0x0C, 0xFA, 0x81, 0x40, 
	0x00, 0x00, 0x40, 0xAE, 0x10, 0x08, 0x04, 0x00, 0x00, 0x00, 0xC0, 0x17, 0xF0, 0x01, 0x7D, 0x00, 
	0x00, 0x02, 0xE1, 0x47, 0x20, 0x10, 0xF0, 0x00, 0x00, 0x00, 0x20, 0x18, 0x0C, 0x47, 0x9D, 0x00, 
	0x00, 0x00, 0x20, 0x18, 0x14, 0x51, 0x10, 0x00, 0x00, 0x00, 0x20, 0x98, 0x4C, 0x56, 0x45, 0x00, 
	0x00, 0x00, 0x20, 0x28, 0x42, 0x88, 0x82, 0x00, 0x00, 0x40, 0x50, 0x44, 0x41, 0x10, 0x04, 0x01, 
	0x00, 0x00, 0xE0, 0x0F, 0x42, 0x08, 0xFE, 0x00, 0x30, 0x04, 0x02, 0x61, 0x40, 0x20, 0x10, 0x30, 
	0x08, 0x04, 0x02, 0x00, 0x40, 0x20, 0x10, 0x00, 0x06, 0x04, 0x02, 0x01, 0x43, 0x20, 0x10, 0x06, 
	0x00, 0x00, 0x40, 0x58, 0xC2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x8E, 0xE8, 0x20, 0x38, 0x88, 0x38, 0x00, 0x98, 0xFF, 0x3F, 0x10, 0x08, 0x04, 0x02, 0x01, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x10, 0x04, 0x30, 0x24, 0x02, 0xC1, 0x41, 0x20, 0x12, 0x06, 
	0x00, 0x00, 0x00, 0x00, 0xB0, 0xD9, 0x48, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA8, 0x00, 0x00, 
	0x08, 0x84, 0x0F, 0x81, 0x40, 0x20, 0x10, 0x08, 0x08, 0x84, 0x0F, 0x81, 0x40, 0xF8, 0x10, 0x08, 
	0x08, 0x8A, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA7, 0xCA, 0x41, 0x70, 0xAF, 0xDE, 0x01, 0x00, 
	0x14, 0x5F, 0x30, 0xE0, 0x03, 0x06, 0x7D, 0x00, 0x00, 0x00, 0x84, 0x21, 0x60, 0x40, 0x00, 0x00, 
	0x00, 0xBE, 0x22, 0x91, 0x4F, 0x28, 0xF8, 0x00, 0x90, 0x64, 0x28, 0x92, 0xB8, 0x84, 0x82, 0x00, 
	0x14, 0xC4, 0x1F, 0x86, 0x61, 0x18, 0xFE, 0x00, 0x22, 0x91, 0x48, 0x24, 0x12, 0x89, 0x7C, 0x08, 
	0x82, 0x9F, 0xC0, 0x23, 0x12, 0x61, 0x00, 0x00, 0x08, 0x02, 0x87, 0x03, 0x00, 0x00, 0x00, 0x00, 
	0x1C, 0x0E, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0xA2, 0xC8, 0xFD, 0x0E, 0x00, 0x00, 0x00, 0x00, 
	0xF7, 0x3B, 0x51, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xC7, 0xC7, 0x41, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x0F, 0x00, 0x00, 0x00, 0x00, 
	0xA6, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x57, 0xB9, 0x5C, 0x0A, 0x00, 0x00, 0x00, 0x00, 
	0x14, 0x04, 0xC0, 0x17, 0xF0, 0x01, 0x7D, 0x00, 0x00, 0x80, 0x80, 0x01, 0x61, 0x08, 0x00, 0x00, 
	0x00, 0x80, 0x2D, 0x99, 0x4B, 0xD8, 0x00, 0x00, 0x90, 0x44, 0x20, 0x18, 0xB9, 0x84, 0x82, 0x00, 
	0x14, 0x04, 0xE0, 0x0F, 0x42, 0x08, 0xFE, 0x00, 0x94, 0xA0, 0x88, 0x82, 0x40, 0x20, 0x10, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x02, 0x87, 0x03, 0x00, 0x00, 0x00, 0x00, 
	0x1C, 0x0E, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x08, 0x0A, 0x81, 0xE0, 0x21, 0x10, 0x3C, 0x00, 
	0x00, 0x8E, 0xE8, 0x20, 0x38, 0x88, 0x38, 0x00, 0xBE, 0x71, 0x20, 0x9D, 0x49, 0x24, 0xD6, 0x3E, 
	0x08, 0x04, 0x02, 0x00, 0x00, 0x20, 0x10, 0x08, 0x9E, 0x40, 0xC0, 0x43, 0xE1, 0x01, 0x83, 0x3E, 
	0x36, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBE, 0x60, 0xB6, 0x58, 0xCC, 0x06, 0x7D, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x0C, 0x00, 0x24, 0x49, 0x92, 0x90, 0x90, 0x90, 0x00, 
	0x00, 0xC0, 0x0F, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x0C, 0x89, 0x84, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xC2, 0x87, 0x40, 0x00, 0x7C, 0x00, 
	0x0C, 0x09, 0x82, 0xE0, 0x01, 0x00, 0x00, 0x00, 0x0E, 0x08, 0x07, 0xE2, 0x00, 0x00, 0x00, 0x00, 
	0x08, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x64, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x82, 0x04, 0x45, 0x14, 0xFC, 0x07, 0x83, 0x00, 0x00, 0x00, 0x80, 0xC1, 0x00, 0x00, 0x00, 0x00, 
	0x80, 0x7E, 0x81, 0xC0, 0x23, 0x10, 0xF8, 0x00, 0x81, 0x22, 0x91, 0xC8, 0x27, 0x12, 0x89, 0x00, 
	0x81, 0x0E, 0x02, 0x81, 0x40, 0x20, 0x38, 0x00, 0x80, 0x84, 0x84, 0x84, 0x24, 0x49, 0x12, 0x00, 
	0x81, 0x9E, 0x50, 0x28, 0x14, 0x0A, 0x79, 0x00, 0x82, 0xA1, 0x48, 0xF2, 0x63, 0x8A, 0x22, 0x78, 
	0x81, 0x22, 0x0A, 0x02, 0x81, 0x40, 0x70, 0x00, 0x81, 0x04, 0x45, 0x24, 0x12, 0x51, 0xEE, 0x00, 
	0x18, 0x25, 0x00, 0x81, 0x40, 0x20, 0x10, 0x1C, 0x00, 0x8E, 0x28, 0x18, 0xFC, 0x07, 0x83, 0x00, 
	0x80, 0x9F, 0x50, 0xE8, 0x13, 0x0A, 0x7F, 0x00, 0x00, 0x3F, 0x91, 0x40, 0x20, 0x10, 0x1C, 0x00, 
	0x00, 0x04, 0x85, 0x22, 0x12, 0x05, 0xFF, 0x00, 0x80, 0xBF, 0x40, 0xE0, 0x13, 0x08, 0xFE, 0x00, 
	0x80, 0x3F, 0x08, 0x82, 0x20, 0x08, 0xFE, 0x00, 0x80, 0x60, 0x30, 0xF8, 0x0F, 0x06, 0x83, 0x00, 
	0x00, 0x5F, 0x30, 0xD8, 0x0D, 0x06, 0x7D, 0x00, 0x00, 0x0E, 0x02, 0x81, 0x40, 0x20, 0x38, 0x00, 
	0x80, 0xB3, 0x44, 0x61, 0x90, 0x88, 0xCE, 0x00, 0x00, 0x04, 0x85, 0x22, 0x12, 0x05, 0x83, 0x00, 
	0x80, 0xE0, 0xB8, 0x9A, 0x0C, 0x06, 0x83, 0x00, 0x80, 0xE0, 0xB0, 0x98, 0x8C, 0x86, 0x83, 0x00, 
	0x80, 0x7F, 0x10, 0xE0, 0x03, 0x04, 0xFF, 0x00, 0x00, 0x5F, 0x30, 0x18, 0x0C, 0x06, 0x7D, 0x00, 
	0x80, 0xBF, 0x48, 0x24, 0x12, 0x89, 0x44, 0x00, 0x80, 0x9F, 0x50, 0xE8, 0x13, 0x08, 0x0E, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x7F, 0xD0, 0x80, 0x30, 0x04, 0xFF, 0x00, 
	0x80, 0x7F, 0x12, 0x81, 0x40, 0x20, 0x38, 0x00, 0x80, 0xA0, 0x88, 0x82, 0x40, 0x20, 0x38, 0x00, 
	0x00, 0x84, 0x2F, 0x99, 0x4C, 0xFA, 0x10, 0x00, 0x80, 0xA0, 0x88, 0x82, 0xA0, 0x88, 0x82, 0x00, 
	0x80, 0x60, 0x32, 0x99, 0xF4, 0x21, 0x10, 0x00, 0x00, 0x8E, 0x28, 0x18, 0x14, 0x51, 0x6C, 0x00, 
	0x14, 0x00, 0x02, 0x81, 0x40, 0x20, 0x10, 0x00, 0x94, 0xA0, 0x88, 0x82, 0x40, 0x20, 0x10, 0x00, 
	0x08, 0x00, 0xC0, 0x1B, 0x0A, 0x85, 0xBC, 0x00, 0x18, 0x00, 0x4F, 0x28, 0xE0, 0x08, 0x84, 0x3C, 
	0x10, 0x80, 0x40, 0xA0, 0x33, 0x0A, 0x85, 0x40, 0x08, 0x00, 0x02, 0x81, 0x40, 0x20, 0x20, 0x00, 
	0x18, 0x25, 0x40, 0x24, 0x14, 0x0A, 0x79, 0x00, 0x00, 0x00, 0xC0, 0x1B, 0x0A, 0x85, 0xBC, 0x00, 
	0x00, 0x00, 0x47, 0xA4, 0x11, 0x89, 0x3A, 0x00, 0x00, 0x80, 0x44, 0x64, 0x22, 0x61, 0x30, 0x0C, 
	0x00, 0x00, 0x4F, 0xC0, 0x13, 0x0A, 0x79, 0x00, 0x00, 0x00, 0x4F, 0x28, 0xE0, 0x08, 0x84, 0x3C, 
	0x00, 0x3E, 0x08, 0x82, 0x20, 0x08, 0xF8, 0x40, 0x00, 0x80, 0x40, 0xA0, 0x33, 0x0A, 0x85, 0x40, 
	0x00, 0x00, 0x47, 0x24, 0xF2, 0x89, 0x44, 0x1C, 0x00, 0x00, 0x02, 0x81, 0x40, 0x20, 0x20, 0x00, 
	0x00, 0x80, 0x99, 0xC2, 0xA0, 0x90, 0x8C, 0x00, 0x00, 0x00, 0x04, 0x02, 0x43, 0x11, 0x85, 0x00, 
	0x00, 0x00, 0x40, 0x24, 0x12, 0x99, 0xB4, 0x02, 0x00, 0x80, 0x44, 0x24, 0x12, 0x49, 0x1C, 0x00, 
	0x7E, 0x18, 0x01, 0x01, 0xE1, 0x09, 0xF8, 0x40, 0x00, 0x00, 0xC0, 0x17, 0x0C, 0x06, 0x7D, 0x00, 
	0x00, 0x00, 0xC0, 0x4F, 0x22, 0x91, 0x48, 0x00, 0x00, 0x00, 0x80, 0x27, 0x34, 0xEA, 0x04, 0x00, 
	0x00, 0x00, 0x80, 0x2F, 0x10, 0x08, 0x78, 0x20, 0x00, 0x00, 0x80, 0x2F, 0x11, 0x89, 0x38, 0x00, 
	0x00, 0x00, 0xC0, 0x87, 0x40, 0x20, 0x30, 0x00, 0x00, 0x00, 0x40, 0x24, 0x14, 0x8A, 0x38, 0x00, 
	0x00, 0x00, 0x20, 0x9F, 0x4C, 0x26, 0x7D, 0x08, 0x00, 0x00, 0x40, 0x48, 0xA2, 0x20, 0x28, 0x62, 
	0x00, 0x00, 0x20, 0x98, 0x4C, 0x26, 0x7D, 0x08, 0x00, 0x00, 0x40, 0x14, 0x4C, 0x26, 0x6D, 0x00, 
	0x14, 0x00, 0x00, 0x81, 0x40, 0x20, 0x20, 0x00, 0x14, 0x00, 0x40, 0x24, 0x14, 0x8A, 0x38, 0x00, 
	0x10, 0x04, 0xC0, 0x17, 0x0C, 0x06, 0x7D, 0x00, 0x10, 0x04, 0x40, 0x24, 0x14, 0x8A, 0x38, 0x00, 
	0x10, 0x04, 0x40, 0x14, 0x4C, 0x26, 0x6D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

