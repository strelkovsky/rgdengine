#pragma once


#define MKFRCC(ch0, ch1, ch2, ch3) ((uint)(uchar)(ch0) | ((uint)(uchar)(ch1) << 8) | ((uint)(uchar)(ch2) << 16) | ((uint)(uchar)(ch3) << 24 )) 


namespace render
{
	enum	texture_usage
	{
		DefaultUsage   =	0x00000000L,
		RenderTarget   =    0x00000001L,
		DepthStencil   =    0x00000002L,
		Dynamic        =    0x00000200L
	};

	enum	texture_format
	{
		R8G8B8               = 20,
		A8R8G8B8             = 21,
		X8R8G8B8             = 22,
		R5G6B5               = 23,
		X1R5G5B5             = 24,
		A1R5G5B5             = 25,
		A4R4G4B4             = 26,
		R3G3B2               = 27,
		A8                   = 28,
		A8R3G3B2             = 29,
		X4R4G4B4             = 30,
		A2B10G10R10          = 31,
		A8B8G8R8             = 32,
		X8B8G8R8             = 33,
		G16R16               = 34,
		A2R10G10B10          = 35,
		A16B16G16R16         = 36,

		A8P8                 = 40,
		P8                   = 41,

		L8                   = 50,
		A8L8                 = 51,
		A4L4                 = 52,

		V8U8                 = 60,
		L6V5U5               = 61,
		X8L8V8U8             = 62,
		Q8W8V8U8             = 63,
		V16U16               = 64,
		A2W10V10U10          = 67,

		//FIXME:
		UYVY                 = MKFRCC('U', 'Y', 'V', 'Y'),
		R8G8_B8G8            = MKFRCC('R', 'G', 'B', 'G'),
		YUY2                 = MKFRCC('Y', 'U', 'Y', '2'),
		G8R8_G8B8            = MKFRCC('G', 'R', 'G', 'B'),
		DXT1                 = MKFRCC('D', 'X', 'T', '1'),
		DXT2                 = MKFRCC('D', 'X', 'T', '2'),
		DXT3                 = MKFRCC('D', 'X', 'T', '3'),
		DXT4                 = MKFRCC('D', 'X', 'T', '4'),
		DXT5                 = MKFRCC('D', 'X', 'T', '5'),

		D16_LOCKABLE         = 70,
		D32                  = 71,
		D15S1                = 73,
		D24S8                = 75,
		D24X8                = 77,
		D24X4S4              = 79,
		D16                  = 80,

		D32F_LOCKABLE        = 82,
		D24FS8               = 83,


		L16                  = 81,

		VERTEXDATA           =100,
		INDEX16              =101,
		INDEX32              =102,

		Q16W16V16U16         =110,

		ARGB8				 = MKFRCC('M','E','T','1'),

		// Floating point surface formats

		// s10e5 formats (16-bits per channel)
		R16F                 = 111,
		G16R16F              = 112,
		A16B16G16R16F        = 113,

		// IEEE s23e8 formats (32-bits per channel)
		R32F                 = 114,
		G32R32F              = 115,
		A32B32G32R32F        = 116,

		CxV8U8               = 117
	};

	enum texture_type
	{
		Unknown       = 1,
		Texture       = 2,
		VolumeTexture = 3,
		CubeTexture   = 4
	};

	typedef boost::shared_ptr<class texture> texture_ptr;

	class texture
	{
	public:
		virtual ~texture(){}

		static texture_ptr		  create(const std::string& filename);

		virtual texture_format get_format() const = 0; 
		virtual texture_usage  get_usage()  const = 0;
		virtual int			  get_height() const = 0;
		virtual int			  width()  const = 0;
		virtual texture_type  type()	  const = 0;
		virtual	bool		  has_alpha()  const = 0;

		virtual const std::string& get_filename() const = 0;
	};
}