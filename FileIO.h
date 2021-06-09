#pragma once
#ifndef FILEIO_TGA_HEADER
#define FILEIO_TGA_HEADER

#ifndef BASE_H
#include "Base.h"
#endif

enum TGATypes
{
	TGA_NODATA = 0,
	TGA_INDEXED = 1,
	TGA_RGB = 2,
	TGA_GRAYSCALE = 3,
	TGA_INDEXED_RLE = 9,
	TGA_RGB_RLE = 10,
	TGA_GRAYSCALE_RLE = 11
};

// Image Data Formats
#define	IMAGE_RGB       0
#define IMAGE_RGBA      1
#define IMAGE_LUMINANCE 2

// Image data types
#define	IMAGE_DATA_UNSIGNED_BYTE 0

//Mis defaults
#define FILEIO_DEF_IMAGEWIDTH 0
#define FILEIO_DEF_IMAGEHEIGHT 0
#define FILEIO_DEF_MAXIMAGEDIM 10000
#define FILEIO_DEF_MINIMAGEDIM 2
#define FILEIO_MAXFILENAMELEN 20

#define FILEIO_ERR_FOPENFAIL 0x10
#define FILEIO_ERR_FWRITEFAIL 0x20
// TGA header
struct tgaheader_t
{
	unsigned char  idLength;
	unsigned char  colorMapType;
	unsigned char  imageTypeCode;
	unsigned char  colorMapSpec[5];
	unsigned short xOrigin;
	unsigned short yOrigin;
	unsigned short width;
	unsigned short height;
	unsigned char  bpp;
	unsigned char  imageDesc;
};


class CTargaImage :public Base {
public:
	CTargaImage();
	~CTargaImage();

	unsigned char Init();
	void Release();
	unsigned char Open(
		unsigned char* pImageData,
		unsigned short width,
		unsigned short height,
		bool           IsBGR = false,
		unsigned char imageDataFormat = IMAGE_RGBA,
		const char* filename = NULL
	);
	unsigned char Open(const char* filename);
	void Close();
	bool Load(const char* filename);
	unsigned char Write(const char* filename, unsigned char bpp = 0);

	/********* Get properties **************************************/
	inline unsigned short GetWidth() { return m_width; }
	inline unsigned short GetHeight() { return m_height; }
	inline unsigned long GetImageSize() { return m_imageSize; }
	inline unsigned char GetImageFormat() { return m_imageDataFormat; }
	inline int GetColorMode() { return m_colorMode; }
	inline unsigned char *GetImage() { return m_pImageData; }

protected:
	bool m_ImageDataIsOwned;

	FILE* pStream_in;
	FILE* pStream_out;

	unsigned short m_xOrigin;
	unsigned short m_yOrigin;
	unsigned char  m_colorDepth;
	unsigned char  m_imageDataType;
	unsigned char  m_imageDataFormat;
	unsigned char  m_imageDesc; //added not in book just to store stuff
	unsigned char* m_pImageData;//this may or may not be owned by the object
	unsigned short m_width;
	unsigned short m_height;
	int            m_colorMode;//bpp bytes(8 bits) per pixel
	unsigned long  m_imageSize;
	char           m_filename[FILEIO_MAXFILENAMELEN];
	unsigned short m_filename_len;

	bool           m_IsBGR;//true if BGR false if RGB

	/************* Helpers to Init/Open ************************/
	bool IsGoodImageDim(unsigned short dim);
	/***********************************************************/
	//utility functions
	unsigned char ConvertRGBAtoRGB();
	unsigned char SwapRedBlue();//swap the red and blue components
	// Flip image vertically
	//bool FlipVerticle();
	inline void SetWidth(unsigned short width) { this->m_width = width; }
	inline void SetHeight(unsigned short height) { this->m_height = height; }
	inline void SetXorigin(unsigned short xorigin) { this->m_xOrigin = xorigin; }
	inline void SetYorigin(unsigned short yorigin) { this->m_yOrigin = yorigin; }
	inline void SetImageDataFormat(unsigned char imageDataFormat) { this->m_imageDataFormat = imageDataFormat; }
	inline void SetImageDesc(unsigned char imageDesc) { this->m_imageDesc = imageDesc; }
	inline bool SetImage(unsigned char** pImageData) {
		if (this->m_imageDataFormat == IMAGE_RGBA) this->m_imageSize = m_height * m_width * 4;
		else this->m_imageSize = m_height * m_width * 3;
		if (this->m_imageSize < 6)
			return false;
		else {
			this->m_pImageData = *pImageData;
			*pImageData = nullptr;
			m_ImageDataIsOwned = true;
			return true;
		}
	}

};
#endif

