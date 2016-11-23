#include <tmos.h>
#include <fonts.h>

const char font10x14_cp1250[4032] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x00, 0x00, 0x00, 
	0x03, 0x0C, 0x00, 0x00, 0xCC, 0x30, 0xC3, 0x0C, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x0C, 0x33, 0xFF, 0xFF, 0xCF, 0x0C, 0x33, 
	0xFF, 0xFF, 0xCF, 0x0C, 0x33, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x8F, 0x3F, 0x33, 0xCC, 0xE0, 
	0x0F, 0x7F, 0x30, 0xC3, 0xCC, 0x1F, 0x3F, 0x30, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0xC2, 0x89, 
	0x19, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x66, 0xE4, 0x90, 0x81, 0x01, 0x00, 0x00, 0x00, 0xC0, 0x80, 
	0x04, 0x12, 0x30, 0xE8, 0xF1, 0x6C, 0xE3, 0x0D, 0x33, 0x8C, 0x7F, 0x3C, 0x03, 0x00, 0x30, 0xC0, 
	0x00, 0x03, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xC0, 0x80, 0x03, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0xE0, 
	0x00, 0x03, 0x0C, 0x70, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 
	0x0C, 0x1C, 0x30, 0x00, 0x00, 0x00, 0x00, 0x43, 0x8C, 0x33, 0xDB, 0xC6, 0x0F, 0x3F, 0xB6, 0xCD, 
	0x1C, 0x23, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0xF0, 0xFF, 0xFF, 
	0x30, 0xC0, 0x00, 0x03, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x03, 0x03, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xF0, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x30, 0x00, 0x00, 0x03, 
	0x0C, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x03, 0x0C, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x03, 0x0C, 0x00, 
	0x00, 0xF0, 0xE3, 0xDF, 0xC0, 0x43, 0x8F, 0x3C, 0xF2, 0xC4, 0x13, 0x2F, 0x3C, 0xB0, 0x7F, 0xFC, 
	0x00, 0x00, 0x00, 0xC0, 0x80, 0x03, 0x0F, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x83, 
	0x7F, 0xFE, 0x01, 0x00, 0x00, 0xF0, 0xE3, 0xDF, 0xC0, 0x01, 0x03, 0x0C, 0x18, 0x30, 0x70, 0x70, 
	0x60, 0xC0, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0xF0, 0xE3, 0xDF, 0xC0, 0x00, 0x03, 0x06, 0x0F, 0x3C, 
	0x80, 0x01, 0x3C, 0xB0, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0x80, 0x03, 0x0F, 0x36, 0xCC, 0x18, 0x33, 
	0xCC, 0xFF, 0xFF, 0x03, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x00, 0x00, 0xFC, 0xFF, 0xFF, 0x00, 0x03, 
	0xFC, 0xF3, 0x1F, 0xC0, 0x00, 0x0F, 0x3C, 0xB0, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0xF0, 0xE3, 0xDF, 
	0x00, 0x03, 0x0C, 0xF0, 0xDF, 0xFF, 0x03, 0x0F, 0x3C, 0xB0, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0xFC, 
	0xFF, 0x3F, 0xC0, 0x80, 0x01, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x80, 0x01, 0x06, 0x00, 0x00, 
	0x00, 0xF0, 0xE3, 0xDF, 0xC0, 0x03, 0x0F, 0xEC, 0x9F, 0x7F, 0x03, 0x0F, 0x3C, 0xB0, 0x7F, 0xFC, 
	0x00, 0x00, 0x00, 0xF0, 0xE3, 0xDF, 0xC0, 0x03, 0x0F, 0xEC, 0x3F, 0xFF, 0x00, 0x03, 0x3C, 0xB0, 
	0x7F, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 
	0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xC0, 0x00, 0x03, 0x06, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x18, 0x30, 0x60, 0xC0, 
	0x00, 0x03, 0x18, 0xC0, 0x00, 0x06, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 
	0xFF, 0x0F, 0x00, 0x00, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 
	0x06, 0x30, 0x80, 0x01, 0x0C, 0x30, 0x60, 0xC0, 0x80, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0xF8, 
	0xF3, 0xFF, 0xC0, 0x00, 0x03, 0x0E, 0x0E, 0x0C, 0x30, 0x00, 0x00, 0x00, 0x0C, 0x30, 0x00, 0x00, 
	0x00, 0xF0, 0xE3, 0xDF, 0xC0, 0x00, 0x03, 0x8C, 0xB1, 0xCF, 0x33, 0xCF, 0x3C, 0xB3, 0x7F, 0xFC, 
	0x00, 0x00, 0x00, 0xC0, 0x80, 0x07, 0x33, 0x86, 0x19, 0x66, 0x98, 0x61, 0xFE, 0x19, 0x66, 0x98, 
	0x61, 0xCF, 0x03, 0x00, 0x00, 0xFC, 0xF3, 0x9F, 0xC1, 0x06, 0x1B, 0xE4, 0x8F, 0x7F, 0x06, 0x1B, 
	0x6C, 0xF0, 0x7F, 0xFF, 0x00, 0x00, 0x00, 0xF0, 0xE3, 0xDF, 0x81, 0x03, 0x0C, 0x30, 0xC0, 0x00, 
	0x03, 0x0C, 0x70, 0xA0, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0xFC, 0xF3, 0x9F, 0xE1, 0x06, 0x1B, 0x6C, 
	0xB0, 0xC1, 0x06, 0x1B, 0x6C, 0xF8, 0x7F, 0xFF, 0x00, 0x00, 0x00, 0xFC, 0xFF, 0xBF, 0x81, 0x06, 
	0x18, 0xE4, 0x9F, 0x7F, 0x06, 0x19, 0x60, 0xE0, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0xFC, 0xFF, 0xBF, 
	0x81, 0x06, 0x18, 0xE2, 0x8F, 0x3F, 0x86, 0x18, 0x60, 0x80, 0x01, 0x0F, 0x00, 0x00, 0x00, 0xF0, 
	0xE3, 0xDF, 0xC1, 0x03, 0x0F, 0x30, 0xC0, 0xF0, 0xC3, 0x0F, 0x7C, 0xB8, 0xFF, 0x7C, 0x03, 0x00, 
	0x00, 0x3C, 0x6F, 0x98, 0x61, 0x86, 0x19, 0xE6, 0x9F, 0x7F, 0x86, 0x19, 0x66, 0x98, 0x61, 0xC7, 
	0x03, 0x00, 0x00, 0xF0, 0x03, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 
	0x0C, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 
	0x36, 0xD8, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0x3C, 0x6C, 0x98, 0x31, 0x66, 0xD8, 0xE0, 0x81, 0x03, 
	0x1E, 0xD8, 0x60, 0x86, 0x31, 0x8F, 0x03, 0x00, 0x00, 0x3C, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 
	0x80, 0x01, 0x06, 0x18, 0x60, 0xE0, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0x0C, 0x3C, 0xF0, 0xE1, 0xCF, 
	0xEF, 0x3D, 0xF3, 0xCC, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x03, 0x00, 0x00, 0x0C, 0x3C, 0xF0, 
	0xC1, 0x0F, 0x6F, 0x3C, 0xF3, 0xD8, 0xC3, 0x0F, 0x3E, 0xF0, 0xC0, 0x03, 0x03, 0x00, 0x00, 0xF0, 
	0xE3, 0xDF, 0xE1, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x7C, 0xB8, 0x7F, 0xFC, 0x00, 0x00, 
	0x00, 0xFC, 0xF3, 0x9F, 0xC1, 0x06, 0x1B, 0xEC, 0x9F, 0x3F, 0x06, 0x18, 0x60, 0x80, 0x01, 0x0F, 
	0x00, 0x00, 0x00, 0xF0, 0xE3, 0xDF, 0xE1, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x8F, 0x7D, 0x9C, 
	0x3F, 0xBC, 0x03, 0x00, 0x00, 0xFC, 0xF3, 0x9F, 0xC1, 0x06, 0x1B, 0xEC, 0xBF, 0x7F, 0x66, 0x18, 
	0x63, 0x98, 0xC1, 0x0F, 0x03, 0x00, 0x00, 0xF0, 0xE3, 0xFF, 0xC0, 0x03, 0x0C, 0xE0, 0x0F, 0x7F, 
	0x00, 0x03, 0x3C, 0xF0, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0xFC, 0xFF, 0x7F, 0x8C, 0x30, 0xC0, 0x00, 
	0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x78, 0x00, 0x00, 0x00, 0x0C, 0x3C, 0xF0, 0xC0, 0x03, 
	0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x7C, 0xB8, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0x0C, 0x3C, 0xF0, 
	0xC0, 0x03, 0x0F, 0x6C, 0x98, 0x61, 0xCC, 0x30, 0x83, 0x07, 0x0C, 0x30, 0x00, 0x00, 0x00, 0x0C, 
	0x3C, 0xF0, 0xC0, 0x33, 0xCF, 0x3C, 0xF3, 0xCC, 0x7B, 0xEB, 0xE5, 0x1C, 0x33, 0xCC, 0x00, 0x00, 
	0x00, 0x0C, 0x3C, 0xB0, 0x61, 0xCC, 0xE0, 0x01, 0x03, 0x0C, 0x78, 0x30, 0x63, 0xD8, 0xC0, 0x03, 
	0x03, 0x00, 0x00, 0x0C, 0x3C, 0xB0, 0x61, 0xCC, 0xE0, 0x01, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 
	0x0C, 0x78, 0x00, 0x00, 0x00, 0xFC, 0xFF, 0x7F, 0xC0, 0x80, 0x01, 0x03, 0x06, 0x0C, 0x18, 0x30, 
	0x60, 0xE0, 0xFF, 0xFF, 0x03, 0x00, 0xF0, 0xC0, 0x03, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 
	0x30, 0xC0, 0x00, 0x03, 0x0C, 0xF0, 0xC0, 0x03, 0x03, 0x0C, 0x60, 0x80, 0x01, 0x0C, 0x30, 0x00, 
	0x03, 0x0C, 0xC0, 0x00, 0x03, 0x18, 0x60, 0x00, 0x03, 0x0C, 0x3C, 0xF0, 0x00, 0x03, 0x0C, 0x30, 
	0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x3C, 0xF0, 0x00, 0x30, 0xE0, 0xC1, 0x8C, 
	0x61, 0x03, 0x07, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x0F, 
	0x30, 0xC0, 0x01, 0x0E, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xF8, 0x07, 0x18, 0x60, 0xFE, 0x0D, 0x36, 0xD8, 
	0xFF, 0xFE, 0x02, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0xF3, 0xEC, 0xF7, 0xF0, 0xC1, 0x03, 0x0F, 
	0x7C, 0xF8, 0x7F, 0xFB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xF8, 0x37, 0xE0, 0x00, 
	0x03, 0x0C, 0x30, 0xA0, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x3C, 0xFB, 0x3D, 
	0xFC, 0xE0, 0x03, 0x0F, 0x3C, 0xB8, 0xFF, 0x7C, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 
	0xF8, 0x37, 0xF0, 0xC0, 0xFF, 0x0F, 0x30, 0xC0, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0xC0, 0xC3, 0x3F, 
	0x83, 0x0C, 0x30, 0xF0, 0xCF, 0x3F, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xFC, 0xFB, 0x7F, 0xC0, 0x00, 0xC3, 0x0F, 0x7C, 0xB0, 0xFF, 0xFC, 0x01, 0x00, 
	0x03, 0x0C, 0x30, 0xC0, 0x00, 0xF3, 0xEC, 0xF7, 0xF0, 0xC1, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 
	0x03, 0x00, 0x00, 0xC0, 0x00, 0x03, 0x00, 0x38, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 
	0x0C, 0x78, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0C, 0x00, 0xE0, 0x01, 0x03, 0x0C, 0x30, 0xC0, 0x00, 
	0x03, 0xCC, 0x30, 0x66, 0xF0, 0x00, 0x00, 0x0C, 0x30, 0xC0, 0x00, 0x83, 0x0D, 0x33, 0xC6, 0x0C, 
	0x1F, 0xCC, 0x30, 0xC6, 0x30, 0x83, 0x01, 0x00, 0x00, 0xF0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 
	0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9B, 
	0xFD, 0x7F, 0xF7, 0xCC, 0x33, 0xCF, 0x3C, 0xF3, 0xC0, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xF3, 0xEC, 0xF7, 0xF0, 0xC1, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x03, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xFC, 0xF8, 0x37, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xB0, 0x7F, 0xFC, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xFB, 0xFC, 0x77, 0xF0, 0xC0, 0x07, 0xFF, 0xB7, 0xCF, 0x00, 0x03, 
	0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0xFB, 0x3F, 0xF8, 0xC0, 0x03, 0x0F, 0xEE, 0x3F, 
	0xDF, 0x00, 0x03, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE6, 0xD9, 0xEF, 0xB1, 0x03, 0x06, 0x18, 
	0x60, 0x80, 0x01, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xFD, 0x3F, 0xE0, 0x00, 
	0xFC, 0x00, 0x1C, 0xF0, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x30, 0xC0, 0x00, 0x03, 0x3F, 0xFC, 0xC0, 
	0x00, 0x03, 0x0C, 0x30, 0xC0, 0x10, 0x7F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 
	0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x7C, 0xB8, 0xFF, 0x7C, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x1B, 0xC6, 0x0C, 0x1E, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0x0F, 0x3C, 0xF3, 0xCC, 0x33, 0xCF, 0xBC, 0xF7, 0xD2, 0xCF, 0x13, 0x02, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1F, 0xEE, 0x1C, 0x3F, 0x30, 0xF0, 0xE3, 0xDC, 0xE1, 0x03, 
	0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x1B, 0x6C, 0x30, 0xC3, 0x98, 0xC1, 0x03, 0x06, 
	0x0C, 0x3E, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x0F, 0x18, 0x30, 0x70, 0x70, 
	0x60, 0xC0, 0xFF, 0xFF, 0x03, 0x00, 0xE0, 0xC0, 0x03, 0x03, 0x0C, 0x30, 0xC0, 0xC0, 0x01, 0x07, 
	0x30, 0xC0, 0x00, 0x03, 0x0C, 0xF0, 0x80, 0x03, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 
	0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x1C, 0xF0, 0x00, 0x03, 0x0C, 0x30, 
	0xC0, 0x00, 0x0E, 0x38, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x3C, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x0C, 0x7A, 0x3C, 0x5B, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xF0, 
	0xC3, 0x0C, 0x33, 0xCC, 0x30, 0xC3, 0x0C, 0x33, 0xCC, 0x30, 0xC3, 0x0C, 0x33, 0xFC, 0xF0, 0x03, 
	0x00, 0x00, 0x00, 0x1F, 0xFE, 0x0C, 0xFE, 0xF0, 0x03, 0x03, 0x3F, 0xFC, 0xC0, 0x20, 0xFE, 0xF0, 
	0x01, 0x00, 0xC0, 0x00, 0xF3, 0xFF, 0xFF, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 
	0x00, 0x03, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x80, 0x01, 0x06, 0x1C, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xC0, 0xF0, 0xCF, 0x3F, 
	0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xC0, 0x18, 0x63, 0xC6, 0x18, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0xCF, 0x0C, 0x30, 0xC0, 0x00, 0x03, 
	0x0C, 0xFF, 0xFF, 0x0F, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x30, 0xC0, 
	0xF0, 0xFF, 0xFF, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0xF0, 0xFF, 0xFF, 0x30, 0xC0, 0x00, 
	0xF0, 0xC0, 0xC3, 0x30, 0xC3, 0x3F, 0xFC, 0xC0, 0x00, 0x03, 0x3F, 0xFC, 0xC0, 0x30, 0xC3, 0xF0, 
	0xC0, 0x03, 0x00, 0x08, 0x58, 0x70, 0x61, 0xC2, 0x80, 0x01, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x51, 
	0xAA, 0xA8, 0x42, 0x04, 0x48, 0xC0, 0x00, 0x00, 0x3F, 0xFE, 0x0F, 0x3C, 0x80, 0x07, 0xFC, 0x80, 
	0x07, 0xF0, 0xC0, 0xFF, 0xF9, 0x03, 0x00, 0x00, 0x00, 0x00, 0x40, 0xC0, 0xC0, 0xC1, 0x81, 0x01, 
	0x1C, 0xC0, 0x01, 0x0C, 0x40, 0x00, 0x00, 0x00, 0xC0, 0x80, 0x01, 0x00, 0x3F, 0xFE, 0x0F, 0x3C, 
	0x80, 0x07, 0xFC, 0x80, 0x07, 0xF0, 0xC0, 0xFF, 0xF9, 0x03, 0x48, 0xC0, 0x00, 0xC0, 0xFF, 0xFF, 
	0xC7, 0x08, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xE0, 0x01, 0x48, 0xC0, 0x00, 0xC0, 
	0xFF, 0xFF, 0x07, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x81, 0xFF, 0xFF, 0x0F, 0x60, 0xC0, 
	0x00, 0xC0, 0xFF, 0xFF, 0x07, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x81, 0xFF, 0xFF, 0x0F, 
	0x0C, 0x30, 0xF0, 0xC3, 0x0F, 0x0C, 0x30, 0xC0, 0x1C, 0xFB, 0x3C, 0x73, 0xCC, 0x30, 0xE3, 0xCC, 
	0x31, 0x03, 0x60, 0xC0, 0x80, 0x01, 0x0E, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x70, 0xC0, 0x01, 0x06, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x33, 0x66, 0x8C, 0x73, 0xCE, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCE, 0x39, 0xC7, 0x98, 0x31, 0x63, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0xF8, 
	0xF0, 0xC7, 0x1F, 0x7F, 0xF8, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xC0, 0x0F, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xBC, 0x48, 0x36, 0xA9, 0x24, 0x92, 0x48, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x48, 0xC0, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF7, 0xFF, 0x80, 0x03, 0xF0, 
	0x03, 0x70, 0xC0, 0xFF, 0xFB, 0x03, 0x00, 0x00, 0x00, 0x80, 0x00, 0x0C, 0xE0, 0x00, 0x0E, 0x60, 
	0xE0, 0xE0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x80, 0x01, 0x00, 0x00, 0xFC, 0xFD, 0x3F, 
	0xE0, 0x00, 0x03, 0xF0, 0x03, 0x70, 0xC0, 0xFF, 0xFB, 0x03, 0xC0, 0x80, 0xC1, 0x00, 0x03, 0x0C, 
	0xFC, 0xF0, 0x03, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x43, 0xFC, 0xE1, 0x03, 0x48, 0xC0, 0x00, 0x00, 
	0x00, 0x00, 0xFC, 0xFF, 0x3F, 0x60, 0xC0, 0xC0, 0xC1, 0x81, 0x01, 0xFF, 0xFF, 0x0F, 0x60, 0xC0, 
	0x00, 0x00, 0x00, 0x00, 0xFC, 0xFF, 0x3F, 0x60, 0xC0, 0xC0, 0xC1, 0x81, 0x01, 0xFF, 0xFF, 0x0F, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x86, 0x31, 0x83, 0x07, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x86, 0x19, 0xC6, 0x0F, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x60, 0x80, 0x01, 0x1E, 0x38, 0x70, 0xC0, 0x01, 
	0x06, 0x18, 0x60, 0xE0, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0x00, 0x30, 0xF0, 0xC0, 0xFC, 0xF0, 0xC3, 
	0x0C, 0x33, 0xFC, 0xF0, 0x33, 0xF0, 0xC0, 0x00, 0x00, 0x00, 0x30, 0xE0, 0xC1, 0x8C, 0x61, 0x86, 
	0x19, 0x66, 0x98, 0x7F, 0x86, 0x19, 0x66, 0xD8, 0xF3, 0x20, 0x00, 0x07, 0x30, 0xC0, 0x00, 0x03, 
	0x0C, 0x30, 0xC0, 0x00, 0x00, 0x00, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x00, 0xF0, 
	0x63, 0x90, 0x00, 0x04, 0xE0, 0xC1, 0x0C, 0x21, 0xCC, 0xE0, 0x01, 0x08, 0x40, 0x81, 0xF9, 0x03, 
	0xCC, 0x30, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFE, 0x1D, 0x3E, 0xF3, 0xC2, 0x0B, 0xCF, 0x7C, 0xB8, 
	0x7F, 0xFC, 0x00, 0x00, 0xFC, 0xF8, 0x3F, 0xF0, 0x00, 0x03, 0xF8, 0xC3, 0x1F, 0xC0, 0x00, 0x0F, 
	0xFC, 0x1F, 0x3F, 0x40, 0xE0, 0x01, 0x00, 0x00, 0x00, 0x33, 0x66, 0xCC, 0x98, 0x31, 0xC3, 0x0C, 
	0x66, 0x30, 0x83, 0x19, 0xCC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 
	0xCF, 0x3F, 0xC0, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xC0, 0x03, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x8F, 
	0x7F, 0x03, 0xEF, 0xBC, 0xF4, 0xCE, 0x4B, 0x2F, 0x3D, 0xB0, 0x7F, 0xFC, 0x00, 0x00, 0x30, 0xC0, 
	0x00, 0xC0, 0xFF, 0xFF, 0x07, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x81, 0xFF, 0xFF, 0x0F, 
	0x30, 0xE0, 0xC1, 0x0C, 0x33, 0xCC, 0xE0, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0C, 0xFC, 0xF0, 0x03, 0x03, 0x0C, 0x00, 0x00, 0xC0, 0x0F, 
	0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 
	0x80, 0x00, 0x01, 0x04, 0xE0, 0x01, 0x00, 0xF0, 0x00, 0x03, 0x0C, 0xB0, 0xC0, 0x01, 0x03, 0x0E, 
	0x30, 0xC0, 0x00, 0x03, 0x0C, 0x78, 0x00, 0x00, 0x60, 0x80, 0x01, 0x03, 0x0C, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xD8, 0x60, 0x83, 
	0x0D, 0x36, 0xD8, 0x60, 0x87, 0x3D, 0xB6, 0xDF, 0xDC, 0x03, 0x0F, 0x00, 0xFC, 0xF3, 0xFF, 0xF3, 
	0xCF, 0x3C, 0xF3, 0x0C, 0x33, 0xCC, 0x30, 0xC3, 0x0C, 0x33, 0xCC, 0x30, 0xC3, 0x0C, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x20, 0x80, 
	0xE0, 0x01, 0x00, 0x00, 0xC0, 0x8F, 0x7F, 0x80, 0x01, 0xE6, 0xDF, 0x60, 0x83, 0xFD, 0xEF, 0x2F, 
	0x10, 0x20, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0x0F, 0x38, 0x00, 0x3F, 0x00, 0x07, 
	0xFC, 0xBF, 0x3F, 0x40, 0xC0, 0x01, 0x00, 0x00, 0x30, 0x83, 0x19, 0xCC, 0x60, 0x06, 0x33, 0xCC, 
	0x98, 0x31, 0x63, 0xC6, 0x0C, 0x00, 0x00, 0x00, 0xC0, 0x3C, 0x63, 0x86, 0x19, 0x06, 0x18, 0x60, 
	0x80, 0x01, 0x06, 0x18, 0x60, 0xE0, 0xFF, 0xFF, 0x03, 0x00, 0xCC, 0x30, 0x63, 0x86, 0x19, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF3, 0x0C, 0x1B, 
	0x6C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x78, 0x00, 0x00, 0x30, 0xC0, 
	0x00, 0x00, 0x00, 0xFF, 0xFF, 0x0F, 0x18, 0x30, 0x70, 0x70, 0x60, 0xC0, 0xFF, 0xFF, 0x03, 0x00, 
	0x60, 0xC0, 0x00, 0xC0, 0x3F, 0xFF, 0x19, 0x6C, 0xB0, 0xC1, 0xFE, 0xFB, 0x67, 0x86, 0x31, 0x86, 
	0x3D, 0x0C, 0x60, 0xC0, 0x00, 0x00, 0x0C, 0x78, 0x30, 0x63, 0x98, 0x61, 0x86, 0xF9, 0x67, 0x98, 
	0x61, 0x86, 0x3D, 0x0F, 0x30, 0x20, 0x01, 0x00, 0x0C, 0x78, 0x30, 0x63, 0x98, 0x61, 0x86, 0xF9, 
	0x67, 0x98, 0x61, 0x86, 0x3D, 0x0F, 0x48, 0xC0, 0x00, 0x00, 0x0C, 0x78, 0x30, 0x63, 0x98, 0x61, 
	0x86, 0xF9, 0x67, 0x98, 0x61, 0x86, 0x3D, 0x0F, 0xCC, 0x30, 0x03, 0x00, 0x0C, 0x78, 0x30, 0x63, 
	0x98, 0x61, 0x86, 0xF9, 0x67, 0x98, 0x61, 0x86, 0x3D, 0x0F, 0x0C, 0x18, 0x00, 0x80, 0x01, 0x06, 
	0x18, 0x60, 0x80, 0x01, 0x06, 0x18, 0x60, 0x80, 0x01, 0xFE, 0xFB, 0x0F, 0x60, 0xC0, 0x00, 0x00, 
	0x3F, 0xFE, 0x1D, 0x3C, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x81, 0xFE, 0xF1, 0x03, 0xFC, 0xF8, 
	0x77, 0xE0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x1C, 0xE8, 0x1F, 0x3F, 0x40, 0xC0, 0x01, 
	0x48, 0xC0, 0x00, 0x00, 0x3F, 0xFE, 0x1D, 0x38, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x81, 0xFE, 
	0xF1, 0x03, 0x60, 0xC0, 0x00, 0xC0, 0xFF, 0xFF, 0x1B, 0x68, 0x80, 0x41, 0xFE, 0x19, 0x64, 0x80, 
	0x81, 0xFF, 0xFF, 0x0F, 0xFF, 0xFF, 0x6F, 0xA0, 0x01, 0x06, 0xF9, 0xE7, 0x9F, 0x41, 0x06, 0x18, 
	0xF8, 0xFF, 0xFF, 0x40, 0x00, 0x07, 0xCC, 0x30, 0x03, 0xC0, 0xFF, 0xFF, 0x1B, 0x68, 0x80, 0x41, 
	0xFE, 0x19, 0x64, 0x80, 0x81, 0xFF, 0xFF, 0x0F, 0x48, 0xC0, 0x00, 0xC0, 0xFF, 0xFF, 0x1B, 0x68, 
	0x80, 0x41, 0xFE, 0x19, 0x64, 0x80, 0x81, 0xFF, 0xFF, 0x0F, 0x60, 0xC0, 0x00, 0x00, 0x1E, 0x30, 
	0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xE0, 0x03, 0x30, 0x20, 0x01, 0x00, 
	0x3F, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xF0, 0x03, 0x48, 0xC0, 
	0x00, 0xC0, 0x3F, 0xFF, 0x19, 0x6E, 0xB0, 0xC1, 0x06, 0x1B, 0x6C, 0xB0, 0xE1, 0xFF, 0xFD, 0x03, 
	0x00, 0xFC, 0xF3, 0x9F, 0xE1, 0x06, 0x1B, 0xFC, 0xF0, 0xC3, 0x06, 0x1B, 0x6C, 0xF8, 0x7F, 0xFF, 
	0x00, 0x00, 0x60, 0xC0, 0x00, 0xC0, 0xC0, 0x03, 0x1F, 0xFC, 0xF0, 0xC6, 0x33, 0x8F, 0x3D, 0xFC, 
	0xE0, 0x03, 0x0F, 0x0C, 0x48, 0xC0, 0x00, 0xC0, 0xC0, 0x03, 0x1F, 0xFC, 0xF0, 0xC6, 0x33, 0x8F, 
	0x3D, 0xFC, 0xE0, 0x03, 0x0F, 0x0C, 0x60, 0xC0, 0x00, 0x00, 0x3F, 0xFE, 0x1D, 0x3E, 0xF0, 0xC0, 
	0x03, 0x0F, 0x3C, 0xF0, 0xE1, 0xFE, 0xF1, 0x03, 0x30, 0x20, 0x01, 0x00, 0x3F, 0xFE, 0x1D, 0x3E, 
	0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xE1, 0xFE, 0xF1, 0x03, 0x98, 0x31, 0x03, 0x00, 0x3F, 0xFE, 
	0x1D, 0x3E, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xE1, 0xFE, 0xF1, 0x03, 0xCC, 0x30, 0x03, 0x00, 
	0x3F, 0xFE, 0x1D, 0x3E, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xE1, 0xFE, 0xF1, 0x03, 0x00, 0x00, 
	0x00, 0x80, 0x40, 0x86, 0x31, 0x83, 0x07, 0x0C, 0x78, 0x30, 0x63, 0x98, 0x40, 0x00, 0x00, 0x00, 
	0x48, 0xC0, 0x00, 0xC0, 0x3F, 0xFF, 0x19, 0x6C, 0xB0, 0xC1, 0xFE, 0xFB, 0x67, 0x86, 0x31, 0x86, 
	0x3D, 0x0C, 0x30, 0x20, 0x81, 0xC4, 0xCC, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 
	0xE1, 0xFE, 0xF1, 0x03, 0x60, 0xC0, 0x00, 0xC0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 
	0x3C, 0xF0, 0xE1, 0xFE, 0xF1, 0x03, 0x98, 0x31, 0x03, 0xC0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 
	0x03, 0x0F, 0x3C, 0xF0, 0xE1, 0xFE, 0xF1, 0x03, 0xCC, 0x30, 0x03, 0xC0, 0xC0, 0x03, 0x0F, 0x3C, 
	0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xF0, 0xE1, 0xFE, 0xF1, 0x03, 0x60, 0xC0, 0x00, 0xC0, 0xC0, 0x03, 
	0x1B, 0xC6, 0x0C, 0x1E, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xE0, 0x01, 0xFF, 0xFF, 0x1F, 0x23, 
	0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x1E, 0x20, 0xE0, 0x00, 0x00, 0xF0, 
	0xE3, 0xDF, 0x60, 0x83, 0x0D, 0xF3, 0xC7, 0x3F, 0xC3, 0x0D, 0x7C, 0xF0, 0x7E, 0xF3, 0x00, 0x00, 
	0x60, 0xC0, 0x00, 0x00, 0x00, 0xE6, 0xD9, 0xEF, 0xB1, 0x03, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 
	0x00, 0x00, 0x60, 0xC0, 0x00, 0x00, 0x00, 0xFC, 0xF8, 0x07, 0x18, 0x60, 0xFE, 0x0D, 0x36, 0xD8, 
	0xFF, 0xFE, 0x02, 0x00, 0x30, 0x20, 0x01, 0x00, 0x00, 0xFC, 0xF8, 0x07, 0x18, 0x60, 0xFE, 0x0D, 
	0x36, 0xD8, 0xFF, 0xFE, 0x02, 0x00, 0x48, 0xC0, 0x00, 0x00, 0x00, 0xFC, 0xF8, 0x07, 0x18, 0x60, 
	0xFE, 0x0D, 0x36, 0xD8, 0xFF, 0xFE, 0x02, 0x00, 0xCC, 0x30, 0x03, 0x00, 0x00, 0xFC, 0xF8, 0x07, 
	0x18, 0x60, 0xFE, 0x0D, 0x36, 0xD8, 0xFF, 0xFE, 0x02, 0x00, 0x60, 0xC0, 0x00, 0x00, 0x0F, 0x30, 
	0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x01, 0x60, 0xC0, 0x00, 0x00, 
	0x00, 0xFC, 0xF8, 0x37, 0xE0, 0x00, 0x03, 0x0C, 0x30, 0xA0, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3F, 0xFE, 0x0D, 0x38, 0xC0, 0x00, 0x03, 0x0C, 0xE8, 0x1F, 0x3F, 0x40, 0xC0, 0x01, 
	0x48, 0xC0, 0x00, 0x00, 0x00, 0xFC, 0xF8, 0x37, 0xE0, 0x00, 0x03, 0x0C, 0x30, 0xA0, 0x7F, 0xFC, 
	0x00, 0x00, 0x60, 0xC0, 0x00, 0x00, 0x00, 0xFC, 0xF8, 0x37, 0xF0, 0xC0, 0xFF, 0x0F, 0x30, 0xC0, 
	0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFE, 0x0D, 0x3C, 0xF0, 0xFF, 0x03, 0x0C, 
	0xF0, 0x3F, 0x3F, 0x40, 0x00, 0x07, 0xCC, 0x30, 0x03, 0x00, 0x00, 0xFC, 0xF8, 0x37, 0xF0, 0xC0, 
	0xFF, 0x0F, 0x30, 0xC0, 0xFF, 0xFC, 0x00, 0x00, 0x48, 0xC0, 0x00, 0x00, 0x00, 0xFC, 0xF8, 0x37, 
	0xF0, 0xC0, 0xFF, 0x0F, 0x30, 0xC0, 0xFF, 0xFC, 0x00, 0x00, 0x60, 0xC0, 0x00, 0x00, 0x00, 0x38, 
	0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x78, 0x00, 0x00, 0x30, 0x20, 0x01, 0x00, 
	0x00, 0x38, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x78, 0x00, 0x00, 0xC0, 0x02, 
	0x0B, 0x0C, 0x30, 0xC0, 0x70, 0xE3, 0xCF, 0x38, 0xC3, 0x0C, 0x33, 0xCC, 0x38, 0xFE, 0x70, 0x03, 
	0xC0, 0x00, 0x03, 0x3F, 0x30, 0xC0, 0x70, 0xE3, 0xCF, 0x38, 0xC3, 0x0C, 0x33, 0xCC, 0x38, 0xFE, 
	0x70, 0x03, 0x60, 0xC0, 0x00, 0x00, 0x00, 0xF3, 0xEC, 0xF7, 0xF0, 0xC1, 0x03, 0x0F, 0x3C, 0xF0, 
	0xC0, 0x03, 0x03, 0x00, 0x48, 0xC0, 0x00, 0x00, 0x00, 0xF3, 0xEC, 0xF7, 0xF0, 0xC1, 0x03, 0x0F, 
	0x3C, 0xF0, 0xC0, 0x03, 0x03, 0x00, 0x60, 0xC0, 0x00, 0x00, 0x00, 0xFC, 0xF8, 0x37, 0xF0, 0xC0, 
	0x03, 0x0F, 0x3C, 0xB0, 0x7F, 0xFC, 0x00, 0x00, 0x30, 0x20, 0x01, 0x00, 0x00, 0xFC, 0xF8, 0x37, 
	0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xB0, 0x7F, 0xFC, 0x00, 0x00, 0x98, 0x31, 0x03, 0x00, 0x00, 0xFC, 
	0xF8, 0x37, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xB0, 0x7F, 0xFC, 0x00, 0x00, 0xCC, 0x30, 0x03, 0x00, 
	0x00, 0xFC, 0xF8, 0x37, 0xF0, 0xC0, 0x03, 0x0F, 0x3C, 0xB0, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x30, 0xC0, 0x00, 0x80, 0x7F, 0xFE, 0x01, 0x00, 0x03, 0x0C, 0x00, 0x00, 0x00, 
	0x48, 0xC0, 0x00, 0x00, 0x00, 0xE6, 0xD9, 0xEF, 0xB1, 0x03, 0x06, 0x18, 0x60, 0x80, 0x01, 0x06, 
	0x00, 0x00, 0x30, 0x20, 0x81, 0x04, 0x0C, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 0x7C, 0xB8, 
	0xFF, 0x7C, 0x03, 0x00, 0x60, 0xC0, 0x00, 0x00, 0x00, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 0x03, 0x0F, 
	0x7C, 0xB8, 0xFF, 0x7C, 0x03, 0x00, 0x98, 0x31, 0x03, 0x00, 0x00, 0x03, 0x0F, 0x3C, 0xF0, 0xC0, 
	0x03, 0x0F, 0x7C, 0xB8, 0xFF, 0x7C, 0x03, 0x00, 0xCC, 0x30, 0x03, 0x00, 0x00, 0x03, 0x0F, 0x3C, 
	0xF0, 0xC0, 0x03, 0x0F, 0x7C, 0xB8, 0xFF, 0x7C, 0x03, 0x00, 0x60, 0xC0, 0x00, 0x00, 0x00, 0x06, 
	0x1B, 0x6C, 0x30, 0xC3, 0x98, 0xC1, 0x03, 0x06, 0x0C, 0x3E, 0x70, 0x00, 0x0C, 0x30, 0xC0, 0xC0, 
	0x0F, 0x3F, 0x30, 0xC0, 0x00, 0x03, 0x0C, 0x30, 0xC4, 0x1F, 0x3E, 0x40, 0xC0, 0x01, 0x00, 0x00, 
	0x00, 0x0E, 0x38, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
