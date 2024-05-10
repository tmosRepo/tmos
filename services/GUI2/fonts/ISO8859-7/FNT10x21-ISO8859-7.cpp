#include <tmos.h>
#include <fonts.h>

const char font10x21_iso8859[6048] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x03, 0x0C, 
	0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x80, 0x07, 0x1E, 0x78, 0x00, 0x8C, 0x31, 0xC6, 0x18, 0x63, 0x8C, 0x31, 0xC6, 0x18, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x8C, 0x31, 0xC6, 0xD8, 0xFF, 0xFF, 0x33, 0xC6, 0x18, 0x63, 
	0x8C, 0x31, 0xC6, 0xD8, 0xFF, 0xFF, 0x33, 0xC6, 0x18, 0x63, 0x8C, 0x01, 0x30, 0xC0, 0x00, 0x03, 
	0xFC, 0xF8, 0xF1, 0x63, 0xC3, 0x0C, 0x36, 0xF0, 0x81, 0x0F, 0x6C, 0x30, 0xC3, 0x0C, 0x33, 0x6E, 
	0xFC, 0xF8, 0x01, 0x03, 0x0C, 0x30, 0x00, 0x00, 0x38, 0xFC, 0xF1, 0x66, 0x9B, 0x6D, 0xB3, 0xCD, 
	0x37, 0x6E, 0x80, 0x01, 0x03, 0x0C, 0x18, 0x60, 0x87, 0x3E, 0xDB, 0x6C, 0x9B, 0x6D, 0xF6, 0xF8, 
	0xC3, 0x01, 0x00, 0xE0, 0xC0, 0x87, 0x3B, 0xC6, 0x18, 0x63, 0x0C, 0x1B, 0x7C, 0xE0, 0xC0, 0x81, 
	0xCF, 0x36, 0x9F, 0x3D, 0xDE, 0x70, 0xC3, 0x8C, 0x77, 0x9F, 0xCF, 0x1C, 0x03, 0x70, 0xC0, 0x01, 
	0x07, 0x1C, 0x70, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x80, 0x01, 0x06, 0x0C, 0x30, 0xC0, 0x80, 
	0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x0C, 0x30, 0xC0, 0x00, 0x06, 
	0x18, 0xC0, 0x00, 0x18, 0xC0, 0x00, 0x03, 0x18, 0x60, 0x80, 0x01, 0x0C, 0x30, 0xC0, 0x00, 0x03, 
	0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x18, 0x60, 0x80, 0x01, 0x03, 0x0C, 0x18, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x0C, 0x30, 0xC0, 0x30, 0xF3, 0xED, 0xB6, 0xF1, 0x83, 0x07, 0x3F, 0xB6, 0xDD, 0x3E, 
	0x33, 0x0C, 0x30, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 
	0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0xC3, 0xFF, 0xFF, 0xC3, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 
	0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x1C, 0x70, 0x80, 0x01, 0x07, 0x0E, 0x18, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0xFF, 0x03, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 
	0x80, 0x03, 0x0E, 0x38, 0x00, 0x80, 0x01, 0x06, 0x18, 0x30, 0xC0, 0x00, 0x03, 0x06, 0x18, 0x60, 
	0xC0, 0x00, 0x03, 0x0C, 0x18, 0x60, 0x80, 0x01, 0x03, 0x0C, 0x30, 0x60, 0x80, 0x01, 0x06, 0x00, 
	0x00, 0xF0, 0xE3, 0x9F, 0x61, 0x87, 0x0F, 0x3C, 0xF0, 0xD0, 0x03, 0x8F, 0x3C, 0xF0, 0xC4, 0x03, 
	0x2F, 0x3C, 0xF0, 0xC0, 0x03, 0x1F, 0x6E, 0x98, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0x01, 0x06, 0x1C, 
	0x78, 0xF0, 0x41, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 
	0x80, 0x01, 0x06, 0x7E, 0xF8, 0x01, 0x00, 0xF0, 0xE3, 0xDF, 0xE1, 0x03, 0x0F, 0x0C, 0x30, 0xC0, 
	0x80, 0x01, 0x07, 0x0C, 0x18, 0x70, 0xC0, 0x80, 0x01, 0x07, 0x0C, 0x18, 0x70, 0xE0, 0xFF, 0xFF, 
	0x03, 0x00, 0xF0, 0xE3, 0xDF, 0xE1, 0x03, 0x0F, 0x0C, 0x30, 0xC0, 0x80, 0xC3, 0x07, 0x0F, 0x60, 
	0x00, 0x03, 0x0C, 0xF0, 0xC0, 0x03, 0x0F, 0x7C, 0xB8, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0x03, 0x0E, 
	0x38, 0xF0, 0xC0, 0x83, 0x0F, 0x36, 0xDC, 0x30, 0xE3, 0x8C, 0x31, 0xC7, 0x0C, 0x33, 0xCC, 0x7F, 
	0xFF, 0x01, 0x03, 0x0C, 0x30, 0xE0, 0x01, 0x00, 0xF8, 0xE7, 0x9F, 0x01, 0x06, 0x18, 0x60, 0x80, 
	0x01, 0xF6, 0xF8, 0xE7, 0xB8, 0xC1, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x03, 0x0F, 0x7C, 0xB8, 0x7F, 
	0xFC, 0x00, 0x00, 0xE0, 0xE3, 0x9F, 0xC1, 0x03, 0x0F, 0x30, 0xC0, 0x00, 0xFB, 0xFC, 0x77, 0xF8, 
	0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x7C, 0xB8, 0x7F, 0xFC, 0x00, 0x00, 0xFC, 0xFF, 
	0xFF, 0xC0, 0x00, 0x03, 0x0E, 0x18, 0x30, 0xC0, 0x00, 0x03, 0x06, 0x18, 0x60, 0x80, 0x01, 0x03, 
	0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0x00, 0x00, 0xF0, 0xE3, 0xDF, 0xE1, 0x03, 0x0F, 0x3C, 
	0xF0, 0xC0, 0x86, 0xF1, 0xE3, 0x9F, 0x61, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x7C, 0xB8, 
	0x7F, 0xFC, 0x00, 0x00, 0xF0, 0xE3, 0xDF, 0xE1, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x7C, 
	0xB8, 0xFF, 0x7C, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x0F, 0x7C, 0xB8, 0x7F, 0xFC, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x70, 0xC0, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xC0, 0x01, 0x07, 0x1C, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x07, 0x1C, 0x70, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x1C, 0x70, 0x80, 0x01, 
	0x07, 0x0E, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x80, 0x03, 0x07, 0x0E, 0x1C, 0x38, 0x70, 
	0xE0, 0xC0, 0x01, 0x0E, 0x70, 0x80, 0x03, 0x1C, 0xE0, 0x00, 0x07, 0x38, 0x40, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x1F, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 
	0xC7, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x07, 
	0x38, 0xC0, 0x01, 0x0E, 0x70, 0x80, 0x03, 0x1C, 0xE0, 0xC0, 0x81, 0x03, 0x07, 0x0E, 0x1C, 0x38, 
	0x70, 0x80, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xE3, 0xDF, 0xE1, 0x03, 0x0F, 0x1C, 0x30, 0xC0, 0x80, 
	0x01, 0x07, 0x0E, 0x18, 0x30, 0xC0, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0C, 0x30, 0x00, 
	0x00, 0xE0, 0xC1, 0x9F, 0x63, 0x06, 0x0F, 0x3C, 0xE6, 0xBC, 0xDB, 0x6E, 0xBB, 0xED, 0xB6, 0xDB, 
	0x6E, 0xBB, 0xFF, 0x6C, 0x03, 0x18, 0xE0, 0x30, 0xFF, 0xF8, 0x00, 0x00, 0xC0, 0x00, 0x03, 0x0C, 
	0x78, 0xE0, 0x81, 0x07, 0x33, 0xCC, 0x30, 0xC3, 0x8C, 0x61, 0x86, 0x19, 0xE6, 0xDF, 0xFF, 0x03, 
	0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x03, 0x00, 0xFC, 0xF1, 0x8F, 0x71, 0x86, 0x19, 0x66, 0x98, 0x61, 
	0x86, 0x19, 0xE3, 0x8F, 0x3F, 0xC6, 0x19, 0x6E, 0xB0, 0xC1, 0x06, 0x1B, 0x6E, 0xDC, 0x7F, 0x7F, 
	0x00, 0x00, 0xF0, 0xE3, 0xDF, 0xE1, 0x03, 0x0F, 0x3C, 0xF0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 
	0x03, 0x0C, 0x30, 0xC0, 0xC0, 0x03, 0x0F, 0x7C, 0xB8, 0x7F, 0xFC, 0x00, 0x00, 0xFC, 0xF1, 0x8F, 
	0x71, 0x86, 0x19, 0x6E, 0xB0, 0xC1, 0x06, 0x1B, 0x6C, 0xB0, 0xC1, 0x06, 0x1B, 0x6C, 0xB0, 0xC1, 
	0x86, 0x1B, 0x66, 0xDC, 0x3F, 0x7F, 0x00, 0x00, 0xFC, 0xFF, 0xBF, 0x81, 0x06, 0x18, 0x60, 0x80, 
	0x01, 0x06, 0x18, 0xE4, 0x9F, 0x7F, 0x06, 0x19, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0xE0, 0xFF, 
	0xFF, 0x03, 0x00, 0xFC, 0xFF, 0xBF, 0xC1, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0xE4, 0x9F, 
	0x7F, 0x06, 0x19, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0xC0, 0x03, 0x0F, 0x00, 0x00, 0xF0, 0xE3, 
	0xDF, 0xE1, 0x03, 0x0F, 0x3C, 0xF0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0xF8, 0xE3, 0x0F, 0x3C, 0xF0, 
	0xE0, 0x83, 0x0F, 0x7F, 0xBE, 0xDF, 0x3C, 0x03, 0x00, 0x0C, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 
	0xF0, 0xC0, 0x03, 0x0F, 0xFC, 0xFF, 0xFF, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 
	0xC0, 0x03, 0x03, 0x00, 0xF8, 0xE7, 0x1F, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 
	0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x83, 0x7F, 0xFE, 0x01, 0x00, 0x00, 
	0x0F, 0x3C, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 
	0x98, 0x61, 0x86, 0x19, 0xC6, 0x1C, 0x3E, 0x70, 0x00, 0x00, 0x3C, 0xFF, 0x98, 0x61, 0xC6, 0x18, 
	0x63, 0x86, 0x19, 0x36, 0xF8, 0xE0, 0x81, 0x0F, 0x36, 0x98, 0x61, 0x86, 0x31, 0xC6, 0x18, 0x66, 
	0xD8, 0xF3, 0xCF, 0x03, 0x00, 0x3C, 0xF0, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 
	0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0xF0, 0xFF, 0xFF, 0x03, 0x00, 
	0x0C, 0x3C, 0xF0, 0xC0, 0x87, 0x1F, 0x7E, 0xF8, 0xF3, 0xCF, 0xEF, 0xBD, 0xF7, 0xCC, 0x33, 0xCF, 
	0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x03, 0x00, 0x18, 0x6C, 0xB0, 0xC3, 0x0E, 
	0x3B, 0xEC, 0xB1, 0xC7, 0x36, 0xDB, 0x6C, 0xB3, 0xD9, 0x66, 0x9B, 0x6D, 0xBC, 0xF1, 0x86, 0x1B, 
	0x6E, 0xB8, 0xC1, 0x06, 0x03, 0x00, 0xF0, 0xE3, 0xDF, 0xE1, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 
	0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x7C, 0xB8, 0x7F, 0xFC, 0x00, 
	0x00, 0xFC, 0xF3, 0x9F, 0xE1, 0x06, 0x1B, 0x6C, 0xB0, 0xC1, 0x06, 0x1B, 0x6C, 0xB8, 0x7F, 0xFE, 
	0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0xC0, 0x03, 0x0F, 0x00, 0x00, 0xF0, 0xE3, 0xDF, 0xE1, 
	0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xCC, 0x73, 
	0x8F, 0x7D, 0x9C, 0x7F, 0xBC, 0x03, 0x00, 0xFC, 0xF3, 0x9F, 0xE1, 0x06, 0x1B, 0x6C, 0xB0, 0xC1, 
	0x06, 0x1B, 0xEE, 0x9F, 0x3F, 0x66, 0x98, 0x61, 0x8C, 0x31, 0xC6, 0x18, 0x66, 0xD8, 0xF3, 0xCF, 
	0x03, 0x00, 0xF0, 0xE3, 0xDF, 0x61, 0x03, 0x0F, 0x3E, 0xC0, 0x00, 0x06, 0x78, 0xC0, 0x03, 0x3C, 
	0xC0, 0x01, 0x06, 0x30, 0xC0, 0x07, 0x0F, 0x6C, 0xB8, 0x7F, 0xFC, 0x00, 0x00, 0xFC, 0xFF, 0x7F, 
	0x8C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 
	0x30, 0xC0, 0x00, 0x03, 0x1E, 0x78, 0x00, 0x00, 0x3C, 0xFF, 0xBC, 0x61, 0x86, 0x19, 0x66, 0x98, 
	0x61, 0x86, 0x19, 0x66, 0x98, 0x61, 0x86, 0x19, 0x66, 0x98, 0x61, 0x86, 0x19, 0xE6, 0x1C, 0x3F, 
	0x78, 0x00, 0x00, 0x3C, 0xFF, 0xBC, 0x61, 0x86, 0x19, 0x66, 0x98, 0x61, 0x86, 0x19, 0xC6, 0x0C, 
	0x33, 0xCC, 0x30, 0x83, 0x07, 0x1E, 0x78, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0x00, 0x00, 0x0C, 0x3C, 
	0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF3, 0xCC, 0x33, 0xCF, 0x3C, 0xF3, 0xDE, 0x4B, 0x2F, 0xBD, 0xB4, 
	0x73, 0xCE, 0x19, 0x66, 0x98, 0x61, 0x86, 0x01, 0x00, 0x0C, 0x3C, 0xB0, 0x61, 0x86, 0x19, 0xC6, 
	0x0C, 0x33, 0x78, 0xE0, 0x01, 0x03, 0x1E, 0x78, 0xE0, 0xC1, 0x0C, 0x33, 0x86, 0x19, 0x36, 0xF0, 
	0xC0, 0x03, 0x03, 0x00, 0x3C, 0xFF, 0xBC, 0x61, 0x86, 0x19, 0x66, 0x18, 0x33, 0xCC, 0x30, 0x83, 
	0x07, 0x1E, 0x78, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x1E, 0x78, 0x00, 0x00, 0xFC, 
	0xF7, 0x5F, 0x30, 0xC0, 0x80, 0x01, 0x06, 0x18, 0x30, 0xC0, 0x00, 0x03, 0x06, 0x18, 0x30, 0xC0, 
	0x00, 0x03, 0x06, 0x18, 0x30, 0xD0, 0x7F, 0xFF, 0x01, 0xF8, 0xE0, 0x83, 0x0F, 0x06, 0x18, 0x60, 
	0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 
	0x0F, 0x3E, 0xF8, 0x00, 0x06, 0x18, 0x60, 0x00, 0x03, 0x0C, 0x30, 0x80, 0x01, 0x06, 0x18, 0xC0, 
	0x00, 0x03, 0x0C, 0x60, 0x80, 0x01, 0x06, 0x30, 0xC0, 0x00, 0x03, 0x18, 0x60, 0x80, 0x01, 0xF8, 
	0xE0, 0x83, 0x0F, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 
	0x03, 0x0C, 0x30, 0xC0, 0x00, 0x83, 0x0F, 0x3E, 0xF8, 0x00, 0x30, 0xE0, 0xC1, 0x8F, 0x73, 0x87, 
	0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x9F, 0x7F, 0x00, 0x00, 
	0x18, 0xE0, 0x00, 0x07, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x80, 0x87, 0x3F, 0xC6, 0x01, 0x06, 0x18, 0x7F, 0xFE, 0x1D, 0x36, 0xD8, 0x60, 0x83, 
	0x0D, 0x77, 0x9E, 0xDF, 0x3C, 0x03, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0xB0, 0xCF, 0x7F, 
	0x87, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x7C, 0xF8, 0x7F, 0xFB, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x8F, 0x7F, 0x87, 0x0F, 0x3C, 0xF0, 0x00, 
	0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0F, 0x7C, 0xB8, 0x7F, 0xFC, 0x00, 0x00, 0x03, 0x0C, 0x30, 
	0xC0, 0x00, 0x03, 0xCC, 0xB7, 0xFF, 0x87, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 
	0x03, 0x0F, 0x7C, 0xB8, 0xFF, 0x7C, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x8F, 
	0x7F, 0x87, 0x0F, 0x3C, 0xF0, 0xC0, 0xFF, 0xFF, 0x3F, 0xC0, 0x00, 0x03, 0x0C, 0x7C, 0xB8, 0x7F, 
	0xFC, 0x00, 0xE0, 0xC3, 0x8F, 0x03, 0x06, 0x18, 0x60, 0xF0, 0xDF, 0x7F, 0x18, 0x60, 0x80, 0x01, 
	0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xC0, 0xB7, 0xFF, 0x87, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x1F, 0xEE, 0x3F, 
	0xDF, 0x00, 0x0F, 0x7C, 0xB8, 0x7F, 0xFC, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 
	0xCF, 0x7E, 0x8F, 0x1F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 
	0xC0, 0x03, 0x03, 0x70, 0xC0, 0x01, 0x07, 0x00, 0x00, 0x00, 0xE0, 0x87, 0x1F, 0x60, 0x80, 0x01, 
	0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x00, 0x70, 0xC0, 
	0x01, 0x07, 0x00, 0x00, 0x00, 0xE0, 0x87, 0x1F, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 
	0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 
	0x30, 0xD8, 0x70, 0xC3, 0x8C, 0x31, 0xC7, 0x0C, 0x3B, 0xFC, 0xF0, 0xC7, 0x19, 0xE3, 0x0C, 0x37, 
	0xD8, 0xE0, 0x03, 0x03, 0x7E, 0xF8, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 
	0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB0, 0xD9, 0xFF, 0x77, 0xCF, 0x3C, 0xF3, 0xCC, 0x33, 0xCF, 
	0x3C, 0xF3, 0xCC, 0x33, 0xCF, 0x3C, 0xF3, 0xCC, 0x33, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x30, 0xCF, 0x7E, 0x8F, 0x1F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 
	0x3C, 0xF0, 0xC0, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x8F, 0x7F, 0x87, 
	0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x7C, 0xB8, 0x7F, 0xFC, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB0, 0xCF, 0x7F, 0x87, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 
	0x0F, 0x7C, 0xF8, 0x7F, 0xFB, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xC0, 0xB7, 0xFF, 0x87, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x7C, 0xB8, 0xFF, 0x7C, 
	0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x9E, 0xFD, 
	0x1E, 0x3B, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x87, 0x3F, 0xC7, 0x0D, 0x36, 0xC0, 0x01, 
	0xFE, 0xF0, 0x07, 0x38, 0xC0, 0x03, 0x0F, 0x7C, 0xB8, 0x7F, 0xFC, 0x00, 0x00, 0x30, 0xC0, 0x00, 
	0x03, 0x0C, 0x30, 0xF0, 0xDF, 0x7F, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 
	0x0C, 0x33, 0xCC, 0x39, 0x7E, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xF0, 
	0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x7E, 0xBC, 0xDF, 
	0x3C, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xF0, 0xC0, 0x03, 0x1B, 0x66, 0x98, 
	0x61, 0xCC, 0x30, 0xC3, 0x0C, 0x12, 0x78, 0xE0, 0x01, 0x03, 0x0C, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x30, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF3, 0xCC, 0x33, 0xCF, 0xBC, 0xF7, 
	0xD2, 0xCF, 0x3B, 0x67, 0x98, 0x61, 0x86, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
	0xF0, 0xE1, 0x86, 0x31, 0xC3, 0x0C, 0x1E, 0x78, 0xC0, 0x80, 0x07, 0x33, 0xCC, 0x18, 0x66, 0xD8, 
	0xC0, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xF0, 0xC0, 0x03, 0x1B, 0x66, 
	0x98, 0x61, 0xCC, 0xE0, 0x81, 0x07, 0x0C, 0x30, 0xC0, 0x80, 0xC1, 0x07, 0x0F, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x9F, 0x7F, 0xC0, 0x00, 0x03, 0x06, 0x18, 0x30, 0xC0, 0x80, 
	0x01, 0x03, 0x0C, 0x18, 0x70, 0xC0, 0x7F, 0xFF, 0x01, 0xE0, 0x80, 0x03, 0x0E, 0x0C, 0x30, 0xC0, 
	0x00, 0x03, 0x0C, 0x30, 0x70, 0xC0, 0x01, 0x07, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 
	0x0E, 0x38, 0xE0, 0x00, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0x00, 
	0x00, 0x00, 0x00, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0x00, 0x38, 
	0xE0, 0x80, 0x03, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x00, 0x07, 0x1C, 0x70, 0x60, 0x80, 
	0x01, 0x06, 0x18, 0x60, 0x80, 0x81, 0x03, 0x0E, 0x38, 0x00, 0x1C, 0xFB, 0x7E, 0xDF, 0x38, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x0E, 0xFE, 0x18, 0x33, 0xC0, 0x00, 0x03, 0x3F, 0xFC, 0xC0, 0x00, 0x03, 0x0C, 
	0xFC, 0xF0, 0x03, 0x03, 0x0C, 0x30, 0x80, 0x31, 0xFE, 0xE0, 0x00, 0x60, 0xC0, 0xF0, 0xFF, 0xFF, 
	0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 
	0x0C, 0x30, 0xC0, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1C, 0x70, 0x80, 0x01, 0x07, 0x0E, 0x18, 
	0x00, 0xC0, 0x80, 0x07, 0x36, 0x8C, 0x30, 0xC0, 0x00, 0x03, 0x3F, 0xFC, 0xC0, 0x00, 0x03, 0x0C, 
	0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC4, 0xB0, 0x81, 0x07, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 
	0xCC, 0x30, 0x83, 0x08, 0x33, 0xCC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xCC, 0x33, 0xCF, 0x0C, 0x00, 0x00, 
	0x00, 0x00, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0xF0, 0xFF, 0xFF, 0x30, 0xC0, 0x00, 0x03, 
	0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0x00, 0x30, 0xC0, 0x00, 
	0x03, 0x0C, 0x30, 0xC0, 0xF0, 0xFF, 0xFF, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0xF0, 0xFF, 
	0xFF, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0x00, 0x30, 0xE0, 0xC1, 0x8C, 0x61, 0x02, 0x01, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x1A, 0x9C, 0x50, 0x22, 0xC9, 0x24, 0x61, 0x02, 0x0C, 0x10, 0x20, 0xC0, 
	0x00, 0x01, 0x02, 0x0C, 0xD0, 0x98, 0x94, 0x52, 0x4A, 0x29, 0x25, 0x63, 0x00, 0x00, 0x84, 0x20, 
	0x01, 0x03, 0x1E, 0xFC, 0x18, 0x36, 0xF0, 0x00, 0x03, 0x0C, 0x60, 0x00, 0x3F, 0xF8, 0x01, 0x0E, 
	0x30, 0xC0, 0x01, 0x0F, 0x6C, 0x18, 0x3F, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0xC0, 0x80, 
	0x01, 0x03, 0x06, 0x0C, 0x18, 0x60, 0x00, 0x03, 0x18, 0xC0, 0x00, 0x06, 0x30, 0x80, 0x01, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xEF, 0xFF, 0x19, 0x63, 0x8C, 0x31, 0xC6, 0x18, 0x63, 0x8C, 
	0x31, 0xFE, 0xF8, 0x63, 0x8C, 0x31, 0xC6, 0x18, 0x63, 0x8C, 0x71, 0x86, 0xFF, 0xFC, 0x03, 0x18, 
	0xCC, 0x36, 0xD8, 0x30, 0xC3, 0x8C, 0x31, 0xC6, 0x0C, 0x33, 0x6C, 0xB0, 0xC1, 0x0F, 0x37, 0x9C, 
	0x31, 0xC6, 0x30, 0xC3, 0x0C, 0x36, 0xD8, 0xC0, 0x03, 0x03, 0x84, 0x20, 0x01, 0xC3, 0xFF, 0xFF, 
	0x03, 0x06, 0x0C, 0x30, 0x60, 0x80, 0x01, 0x03, 0x0C, 0x18, 0x60, 0xC0, 0x00, 0x03, 0x06, 0x18, 
	0x30, 0xC0, 0xFF, 0xFF, 0x03, 0x00, 0x18, 0x66, 0x98, 0x61, 0x86, 0x19, 0x66, 0x98, 0x61, 0x86, 
	0x19, 0x66, 0x98, 0x61, 0x86, 0x19, 0xE6, 0x9F, 0x7F, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0x00, 
	0x00, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0xFC, 0xF1, 0x07, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0xCC, 
	0xB1, 0xCF, 0x33, 0xC7, 0x0C, 0x33, 0xCC, 0x30, 0x63, 0xCC, 0x00, 0x60, 0xC0, 0x81, 0x03, 0x06, 
	0x38, 0xE0, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0xC0, 0x01, 0x07, 0x18, 0x70, 0xE0, 0x80, 0x01, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x18, 0x73, 0xEE, 0x9C, 0x31, 0xCE, 0x39, 0xE7, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCE, 0x39, 0xE7, 0x1C, 
	0x63, 0xC6, 0x9D, 0x33, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xC0, 0x80, 0x07, 0x3F, 0xFC, 0xF0, 0x83, 0x07, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x1E, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0xFF, 0x03, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x39, 0xF7, 
	0xCF, 0x1C, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x2F, 0xBE, 0x6D, 0xBE, 0xF9, 0xE6, 0x9B, 0x6D, 0xB6, 0xD9, 0x66, 0x9B, 0x0D, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCC, 0xE0, 
	0x01, 0x03, 0x00, 0x00, 0x00, 0xC0, 0x87, 0x3F, 0xC7, 0x0D, 0x36, 0xC0, 0x01, 0xFE, 0xF0, 0x07, 
	0x38, 0xC0, 0x03, 0x0F, 0x7C, 0xF8, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x0C, 0x60, 
	0x00, 0x03, 0x18, 0xC0, 0x00, 0x06, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x01, 0x03, 0x06, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x8C, 0x7F, 0x73, 0xCF, 
	0x38, 0xE3, 0x8C, 0xF3, 0xCF, 0x3F, 0xC3, 0x0C, 0x33, 0xCC, 0x3C, 0xBF, 0x7F, 0xCC, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x20, 0xC0, 0xB0, 0xD9, 0x70, 0xC3, 0x8C, 0x31, 0xC7, 0x0C, 0x3B, 0xFC, 
	0xF0, 0xC7, 0x19, 0xE3, 0x0C, 0x37, 0xD8, 0xE0, 0x03, 0x03, 0xCC, 0xE0, 0x01, 0x03, 0x00, 0x00, 
	0x00, 0xE0, 0x9F, 0x7F, 0xC0, 0x00, 0x03, 0x06, 0x18, 0x30, 0xC0, 0x80, 0x01, 0x03, 0x0C, 0x18, 
	0x70, 0xC0, 0x7F, 0xFF, 0x01, 0x48, 0x2C, 0x3D, 0xF0, 0xC0, 0x86, 0x19, 0x66, 0x18, 0x33, 0xCC, 
	0x30, 0x83, 0x07, 0x1E, 0x78, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0xC0, 0x81, 0x03, 0x06, 
	0x38, 0xE0, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0xC0, 0x01, 0x07, 0x18, 0x70, 0xE0, 0x80, 0x01, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x20, 0xC0, 0x81, 0x0F, 0x77, 0x8C, 0x31, 0xC6, 0x00, 0x03, 0x18, 0xF8, 0xE1, 0x87, 0x1F, 
	0x18, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xF0, 0xDF, 0x7F, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x0E, 
	0xFE, 0x18, 0x33, 0xC0, 0x00, 0x03, 0x3F, 0xFC, 0xC0, 0x00, 0x03, 0x0C, 0xFC, 0xF0, 0x03, 0x03, 
	0x0C, 0x30, 0x80, 0x31, 0xFE, 0xE0, 0x00, 0x00, 0xF0, 0xE3, 0xDF, 0xC0, 0x03, 0x0C, 0x30, 0xC0, 
	0x00, 0x33, 0xCF, 0x3E, 0xCF, 0x1C, 0x33, 0xCC, 0x30, 0xC3, 0x0C, 0x33, 0xCC, 0x30, 0xF3, 0x7F, 
	0xFC, 0x00, 0x00, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x38, 0xF8, 0xF0, 0xE1, 0x80, 0x1D, 0xFF, 0xF7, 0x98, 0x43, 0x1C, 0xE1, 0x8C, 0xE3, 0x3F, 
	0xFE, 0x80, 0x01, 0x07, 0x0F, 0x1F, 0x1C, 0x00, 0x00, 0x9C, 0x73, 0xCE, 0x39, 0xE7, 0x9C, 0x03, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xF0, 0xE3, 0xDF, 0xE1, 0x03, 0x8F, 0x3C, 0xF7, 0xC4, 0x13, 0x4F, 0x3C, 
	0xF1, 0xDC, 0x23, 0x0F, 0x7C, 0xB8, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x06, 0x18, 0x60, 0x80, 0x01, 0x1E, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x22, 0xCC, 0x98, 0x31, 0x63, 0xC6, 0x0C, 0x66, 0x30, 0x83, 0x19, 0xCC, 0x20, 0x02, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFC, 0xF3, 0x0F, 0x30, 0xC0, 0x00, 
	0x03, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xC0, 0x03, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xE3, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x18, 0xF0, 0x60, 0x86, 0x19, 0x66, 0xF0, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 
	0x30, 0xC0, 0xE0, 0x9F, 0x7F, 0xFE, 0xC1, 0x00, 0x03, 0x0C, 0x00, 0x00, 0xE0, 0x9F, 0x7F, 0xFE, 
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xF0, 0xE0, 0x87, 0x19, 0x62, 0xC0, 0x81, 0x03, 0x07, 
	0x0E, 0xF8, 0xE1, 0x87, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x3C, 0xF8, 0x21, 0x06, 0x18, 0x60, 0xF0, 0xC1, 0x07, 0x18, 0x60, 0x88, 0xE1, 0x07, 0x0F, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0xC0, 0x80, 
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xC6, 0xCC, 0x1B, 0x03, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0xCC, 0x30, 0x43, 0x0C, 0x78, 0xE0, 0x81, 0x07, 0x33, 0xCC, 0x30, 0xC3, 0x8C, 
	0x61, 0x86, 0x19, 0xE6, 0xDF, 0xFF, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x03, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x80, 0x03, 0x0E, 0x38, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF4, 0xDF, 0x7F, 0x86, 0x19, 0x60, 0x80, 
	0x01, 0x06, 0x18, 0x60, 0x84, 0x1F, 0x7E, 0x18, 0x61, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x21, 
	0xFF, 0xFC, 0x03, 0x03, 0x6C, 0xBC, 0x71, 0xC6, 0x18, 0x63, 0x8C, 0x31, 0xC6, 0x18, 0x63, 0x8C, 
	0x3F, 0xFE, 0x18, 0x63, 0x8C, 0x31, 0xC6, 0x18, 0x63, 0x8C, 0x31, 0xC6, 0x18, 0x03, 0xFB, 0xED, 
	0x37, 0x46, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 
	0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x7E, 0xF8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x10, 0xC1, 0x0C, 0x66, 0x30, 0x83, 0x19, 0xCC, 0x98, 0x31, 0x63, 0xC6, 0x0C, 0x11, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xE3, 0xCC, 0xB7, 0x71, 0xC6, 0x18, 0x63, 0x8C, 0x31, 0xC6, 0x18, 0x63, 
	0x8C, 0x31, 0xC6, 0x18, 0x63, 0x8C, 0x31, 0xC6, 0x18, 0x63, 0x8C, 0x31, 0x7C, 0xE0, 0x00, 0x0C, 
	0x38, 0xF0, 0xC0, 0x03, 0x0C, 0x30, 0xC0, 0x10, 0x63, 0xFF, 0xFC, 0x01, 0x03, 0x06, 0xCC, 0x99, 
	0x2F, 0x32, 0xC0, 0x80, 0x01, 0x03, 0x3E, 0xF8, 0x00, 0x00, 0x01, 0x34, 0xDC, 0x70, 0xC3, 0x0C, 
	0x33, 0xCC, 0x30, 0x66, 0x98, 0x61, 0x06, 0x0F, 0x3C, 0xF0, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 
	0x01, 0x06, 0x3C, 0xF0, 0x00, 0xF1, 0xE4, 0xD7, 0x30, 0xC3, 0x0C, 0x33, 0xCC, 0x30, 0xC3, 0x0C, 
	0x33, 0xCC, 0x30, 0xC3, 0x0C, 0x33, 0xCC, 0x30, 0x66, 0x90, 0x40, 0x82, 0x19, 0xE7, 0x9C, 0x03, 
	0x30, 0xD8, 0x66, 0x99, 0x61, 0x00, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 
	0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x1E, 0x78, 0x00, 0x00, 0xC0, 0x00, 0x03, 0x0C, 
	0x78, 0xE0, 0x81, 0x07, 0x33, 0xCC, 0x30, 0xC3, 0x8C, 0x61, 0x86, 0x19, 0xE6, 0xDF, 0xFF, 0x03, 
	0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x03, 0x00, 0xFC, 0xF3, 0x9F, 0x71, 0x86, 0x19, 0x66, 0x98, 0x61, 
	0x86, 0x19, 0xE3, 0x8F, 0x3F, 0xC6, 0x19, 0x6E, 0xB0, 0xC1, 0x06, 0x1B, 0x6C, 0xFC, 0x7F, 0x7F, 
	0x00, 0x00, 0xFC, 0xFF, 0xBF, 0xC1, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 
	0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0xC0, 0x03, 0x0F, 0x00, 0x30, 0xC0, 0x00, 0x03, 
	0x1E, 0x48, 0x30, 0xC3, 0x0C, 0x33, 0x86, 0x19, 0x66, 0x98, 0x61, 0x86, 0x19, 0x36, 0xF0, 0xC0, 
	0x03, 0x0F, 0x3C, 0xF0, 0xFF, 0xFF, 0x03, 0x00, 0xFC, 0xFF, 0xBF, 0x81, 0x06, 0x18, 0x60, 0x80, 
	0x01, 0x06, 0x18, 0xE4, 0x9F, 0x7F, 0x06, 0x19, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0xE0, 0xFF, 
	0xFF, 0x03, 0x00, 0xFC, 0xFF, 0x7F, 0xC0, 0x80, 0x01, 0x06, 0x0C, 0x30, 0x60, 0x80, 0x01, 0x03, 
	0x0C, 0x18, 0x60, 0xC0, 0x00, 0x03, 0x06, 0x18, 0x30, 0xE0, 0xFF, 0xFF, 0x03, 0x00, 0x0C, 0x3C, 
	0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0xFC, 0xFF, 0xFF, 0x03, 0x0F, 0x3C, 0xF0, 
	0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x03, 0x00, 0xE0, 0xE1, 0x9F, 0x61, 0x03, 0x0F, 0x3C, 
	0xF0, 0xC0, 0x03, 0x0F, 0xBC, 0xF7, 0xDE, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x6C, 0x98, 
	0x7F, 0x78, 0x00, 0x00, 0xF0, 0xC3, 0x0F, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 
	0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x3F, 0xFC, 0x00, 0x00, 0x3C, 
	0xFF, 0x98, 0x61, 0xC6, 0x18, 0x63, 0x86, 0x19, 0x36, 0xF8, 0xE0, 0x81, 0x0F, 0x36, 0x98, 0x61, 
	0x86, 0x31, 0xC6, 0x18, 0x66, 0xD8, 0xF3, 0xCF, 0x03, 0x00, 0xC0, 0x00, 0x03, 0x0C, 0x78, 0x20, 
	0xC1, 0x0C, 0x33, 0xCC, 0x30, 0xC3, 0x0C, 0x33, 0xCC, 0x18, 0x66, 0x98, 0x61, 0x86, 0x19, 0x66, 
	0xD8, 0xF3, 0xCF, 0x03, 0x00, 0x0C, 0x3C, 0xF0, 0xC0, 0x87, 0x1F, 0x7E, 0xF8, 0xF3, 0xCF, 0xEF, 
	0xBD, 0xF7, 0xCC, 0x33, 0xCF, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x03, 0x00, 
	0x18, 0x6C, 0xB0, 0xC3, 0x0E, 0x3B, 0xEC, 0xB1, 0xCF, 0x36, 0xDB, 0x6C, 0xB3, 0xD9, 0x66, 0x9B, 
	0x6D, 0xBC, 0xF1, 0x86, 0x1B, 0x6E, 0xB8, 0xC1, 0x06, 0x03, 0x00, 0xFC, 0xFF, 0xFF, 0xC0, 0x03, 
	0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x0F, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 
	0x3C, 0xF0, 0xFF, 0xFF, 0x03, 0x00, 0xF0, 0xE3, 0xDF, 0xE1, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 
	0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x7C, 0xB8, 0x7F, 0xFC, 0x00, 
	0x00, 0xFC, 0xFF, 0xBF, 0x61, 0x86, 0x19, 0x66, 0x98, 0x61, 0x86, 0x19, 0x66, 0x98, 0x61, 0x86, 
	0x19, 0x66, 0x98, 0x61, 0x86, 0x19, 0x66, 0x98, 0x61, 0x86, 0x01, 0x00, 0xFC, 0xF3, 0x9F, 0xE1, 
	0x06, 0x1B, 0x6C, 0xB0, 0xC1, 0x06, 0x1B, 0xEE, 0x9F, 0x3F, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 
	0x18, 0x60, 0xC0, 0x03, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xFC, 0xFF, 0xFF, 0xC0, 0x06, 0x18, 0xC0, 0x00, 0x03, 0x18, 0x60, 0x00, 0x03, 0x0C, 
	0x18, 0x60, 0xC0, 0x00, 0x03, 0x06, 0x18, 0x30, 0xF0, 0xFF, 0xFF, 0x03, 0x00, 0xFC, 0xFF, 0x7F, 
	0x8C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 
	0x30, 0xC0, 0x00, 0x03, 0x1E, 0x78, 0x00, 0x00, 0x3C, 0x6F, 0x98, 0x61, 0x86, 0x19, 0xC6, 0x0C, 
	0x33, 0xCC, 0xE0, 0x81, 0x07, 0x1E, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x1E, 
	0x78, 0x00, 0x00, 0xE0, 0x01, 0x03, 0x0C, 0x30, 0xF8, 0xF7, 0xFF, 0xCC, 0x33, 0xCF, 0x3C, 0xF3, 
	0xCC, 0x33, 0xCF, 0x3C, 0xF3, 0xFF, 0xFE, 0xC1, 0x00, 0x03, 0x0C, 0x78, 0x00, 0x00, 0x0C, 0x3C, 
	0xF0, 0xC0, 0x86, 0x19, 0x66, 0x18, 0x33, 0xCC, 0x30, 0x03, 0x03, 0x0C, 0xCC, 0x30, 0xC3, 0x8C, 
	0x61, 0x86, 0x19, 0x36, 0xF0, 0xC0, 0x03, 0x03, 0x00, 0x0C, 0x3C, 0xF3, 0xCC, 0x33, 0xCF, 0x3C, 
	0xF3, 0xCC, 0x33, 0xCF, 0x3C, 0xF3, 0xED, 0xFE, 0xF1, 0x03, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 
	0x1E, 0x78, 0x00, 0x00, 0xF0, 0xE3, 0xDF, 0xE1, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 
	0xF0, 0xC0, 0x03, 0x0F, 0x6C, 0x18, 0x33, 0x48, 0x20, 0xC1, 0xCC, 0xF3, 0xCF, 0x03, 0xCC, 0x30, 
	0xC3, 0x0C, 0x00, 0x00, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 
	0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0x00, 0xCC, 0x30, 0xC3, 0x0C, 0x00, 0x01, 0x06, 
	0x38, 0xF0, 0xC0, 0x86, 0x19, 0xC6, 0x0C, 0x3F, 0x78, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 
	0x03, 0x0C, 0x30, 0x00, 0x60, 0xC0, 0x80, 0x01, 0x00, 0x00, 0x00, 0xC0, 0xA3, 0xDF, 0xC7, 0x0D, 
	0x36, 0xD8, 0x60, 0x83, 0x0D, 0x36, 0xD8, 0x60, 0x83, 0x0D, 0x76, 0x9C, 0xDF, 0x3C, 0x02, 0x60, 
	0xC0, 0x80, 0x01, 0x00, 0x00, 0xE0, 0xE1, 0xDF, 0xC1, 0x03, 0x0E, 0x30, 0x80, 0x01, 0x7C, 0xF0, 
	0x61, 0xC0, 0x00, 0x03, 0x0C, 0x78, 0xB0, 0x7F, 0x78, 0x00, 0x60, 0xC0, 0x80, 0x01, 0x00, 0x00, 
	0x00, 0x60, 0x8E, 0x7D, 0x1E, 0x3B, 0x6C, 0xB0, 0xC1, 0x06, 0x1B, 0x6C, 0xB0, 0xC1, 0x06, 0x1B, 
	0x0C, 0x30, 0xC0, 0x00, 0x03, 0x60, 0xC0, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x30, 
	0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x1C, 0x60, 0x00, 
	0x30, 0xD8, 0x66, 0x99, 0x61, 0x00, 0x00, 0x60, 0x98, 0xC1, 0x06, 0x1B, 0x6C, 0xB0, 0xC1, 0x06, 
	0x1B, 0x6C, 0xB0, 0xC1, 0x06, 0x1B, 0xEC, 0x38, 0x7F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xC0, 0xA3, 0xDF, 0xC7, 0x0D, 0x36, 0xD8, 0x60, 0x83, 0x0D, 0x36, 0xD8, 0x60, 0x83, 
	0x0D, 0x76, 0x9C, 0xDF, 0x3C, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x07, 0x3F, 
	0x86, 0x19, 0x64, 0x90, 0x61, 0xF6, 0xD8, 0x63, 0x98, 0x41, 0x06, 0x19, 0xE6, 0x8C, 0x3E, 0x73, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x8C, 0x31, 0x86, 0x31, 0xCC, 0x30, 0xC6, 
	0x18, 0xC3, 0x06, 0x1F, 0x38, 0x60, 0xC0, 0x00, 0x03, 0x07, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xC0, 0x8F, 0x3F, 0x06, 0x30, 0x80, 0x01, 0x1F, 0xFE, 0x1C, 0x37, 0xD8, 0x60, 
	0x83, 0x0D, 0x76, 0x9C, 0x3F, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x87, 
	0x7F, 0x07, 0x0F, 0x38, 0x80, 0x01, 0x7C, 0xF0, 0x61, 0xC0, 0x00, 0x03, 0x0C, 0x70, 0xB0, 0x7F, 
	0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xF3, 0x0F, 0x18, 0x38, 0x30, 0x60, 0xC0, 0x80, 
	0x01, 0x06, 0x18, 0xE0, 0x00, 0x0F, 0xF0, 0x01, 0x0E, 0x30, 0xC0, 0x80, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x60, 0x80, 0x01, 0xE6, 0xD9, 0xEF, 0xB1, 0xC3, 0x06, 0x1B, 0x6C, 0xB0, 
	0xC1, 0x06, 0x1B, 0x6C, 0x30, 0xC0, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 
	0x07, 0x3F, 0xCE, 0x19, 0x66, 0x98, 0x61, 0xFE, 0xF9, 0x67, 0x98, 0x61, 0x86, 0x19, 0xE6, 0x1C, 
	0x3F, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 
	0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x60, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x80, 0x83, 0x0C, 0x33, 0xC6, 0x0C, 0x1B, 0x3C, 0x70, 0xC0, 
	0x03, 0x0B, 0x6C, 0x30, 0xC3, 0x98, 0xC3, 0x0E, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x03, 0x0C, 0x30, 0x80, 0x01, 0x06, 0x18, 0xC0, 0x00, 0x03, 0x0C, 0x78, 0xB0, 0x61, 0xC6, 
	0xB0, 0xC1, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x61, 0x86, 0x19, 
	0x66, 0x98, 0x61, 0x86, 0x19, 0xE6, 0x9C, 0x73, 0xFE, 0xD9, 0x66, 0x98, 0xE1, 0x06, 0x03, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC1, 0x06, 0x1B, 0xCC, 0x18, 0x63, 0x8C, 0x61, 
	0x83, 0x0D, 0x36, 0x70, 0xC0, 0x01, 0x07, 0x08, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFC, 
	0x3B, 0x60, 0x00, 0x07, 0xF8, 0xE3, 0xCF, 0x80, 0x01, 0x06, 0x18, 0xE0, 0x01, 0x3F, 0xE0, 0x03, 
	0x0C, 0x30, 0x70, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0xFC, 
	0x39, 0x6E, 0xB0, 0xC1, 0x06, 0x1B, 0x6C, 0xB0, 0xC1, 0x06, 0x1B, 0xEC, 0x38, 0x7F, 0xF8, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xFF, 0xFE, 0x33, 0xC6, 0x18, 0x63, 0x8C, 
	0x31, 0xC6, 0x18, 0x63, 0x8C, 0x31, 0xC6, 0x18, 0x63, 0x8C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x3E, 0xFC, 0x39, 0x6E, 0xB0, 0xC1, 0x06, 0x1B, 0xEC, 0xB8, 0x7F, 0xF6, 
	0x18, 0x60, 0x80, 0x01, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 
	0xFC, 0x3B, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0x0E, 0xF0, 0x83, 0x1F, 0x60, 0xC0, 
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFC, 0x1B, 0x63, 0x98, 0x61, 
	0x86, 0x19, 0x66, 0x98, 0x61, 0x86, 0x19, 0x66, 0x18, 0x3F, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x80, 0x7F, 0xFE, 0xC1, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 
	0x30, 0xC0, 0x00, 0x03, 0x1C, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 
	0x31, 0xC6, 0x18, 0x66, 0x98, 0xC1, 0x06, 0x1B, 0x6C, 0xB0, 0xC1, 0x06, 0x1B, 0x6E, 0x98, 0x7F, 
	0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x79, 0xF3, 0xCF, 0x3C, 0xF3, 
	0xCC, 0x33, 0xCF, 0x3C, 0xF3, 0xCC, 0x33, 0xFB, 0xC7, 0x0F, 0x0C, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC1, 0x0E, 0x2B, 0x86, 0x19, 0x36, 0xF0, 0x80, 0x00, 0x07, 
	0x36, 0xD8, 0x30, 0xC6, 0x98, 0xC1, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xC0, 0xC0, 0x03, 0x0F, 0x3C, 0xF3, 0xCC, 0x33, 0xCF, 0x3C, 0xF3, 0xCC, 0xFF, 0xFB, 0x07, 0x03, 
	0x0C, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x61, 0x87, 0x0F, 0x3C, 
	0xF0, 0xC0, 0x33, 0xCF, 0x3C, 0xF3, 0xCC, 0x33, 0xFF, 0xEF, 0x1F, 0x33, 0xCC, 0x00, 0xCC, 0x30, 
	0xC3, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 
	0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x60, 0x00, 0xCC, 0x30, 0xC3, 0x0C, 0x00, 0x00, 0x00, 
	0x00, 0x80, 0x31, 0xC6, 0x18, 0x66, 0x98, 0xC1, 0x06, 0x1B, 0x6C, 0xB0, 0xC1, 0x06, 0x1B, 0x6E, 
	0x98, 0x7F, 0xFC, 0x00, 0x60, 0xC0, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0xFC, 0x39, 
	0x6E, 0xB0, 0xC1, 0x06, 0x1B, 0x6C, 0xB0, 0xC1, 0x06, 0x1B, 0xEC, 0x38, 0x7F, 0xF8, 0x00, 0x00, 
	0x00, 0x03, 0x06, 0x0C, 0x00, 0x00, 0x00, 0x80, 0x31, 0xC6, 0x18, 0x66, 0x98, 0xC1, 0x06, 0x1B, 
	0x6C, 0xB0, 0xC1, 0x06, 0x1B, 0x6E, 0x98, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0x03, 0x06, 0x0C, 0x00, 
	0x00, 0x00, 0x80, 0x61, 0x87, 0x0F, 0x3C, 0xF0, 0xC0, 0x33, 0xCF, 0x3C, 0xF3, 0xCC, 0x33, 0xFF, 
	0xEF, 0x1F, 0x33, 0xCC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

