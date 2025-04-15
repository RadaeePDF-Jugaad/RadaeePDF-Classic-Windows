#pragma once

#include "PDFWinRT.h"
#include <stdlib.h>
#include <windows.h>

using namespace Platform;
using namespace Windows::Graphics::Imaging;
char *cvt_str_cstr( String ^str );
String ^cvt_cstr_str( const char *str );

namespace RDPDFLib
{
	namespace pdf
	{
		ref class PDFMatrix;
		ref class PDFOutline;
		ref class PDFPage;
		ref class PDFAnnot;
		ref class PDFDocImage;
		ref class PDFDocFont;
		ref class PDFDocForm;
		ref class PDFDocGState;
		ref class PDFSign;
		ref class PDFImportCtx;
		ref class PDFPageContent;
		ref class PDFPageForm;
		ref class PDFPageFont;
		ref class PDFPageImage;
		ref class PDFPageGState;
		ref class PDFDoc;
		ref class PDFBmp;
		public enum class PDF_ERROR
		{
			err_ok,
			err_invalid_para,
			err_open,
			err_password,
			err_encrypt,
			err_bad_file,
		};
		public enum class PDF_RENDER_MODE
		{
			mode_poor = 0,
			mode_normal = 1,
			mode_best = 2,
		};
		public value struct PDFPoint
		{
			float x;
			float y;
		};
		public value struct PDFRect
		{
			float left;
			float top;
			float right;
			float bottom;
		};
		public value struct PDFRef
		{
			unsigned long long ref;
		};
		public value struct PDFTextRet
		{
			int num_unicodes;
			int num_lines;
		};
		public ref class PDFGlobal sealed
		{
		public:
			/// <summary>
			/// Set paths of cmap and umap resources
			/// </summary>
			/// <param name="cpath">Path of cmap</param>
			/// <param name="upath">Path of umap</param>
			static void SetCMapsPath( String ^cpath, String ^upath )
			{
				char *cp = cvt_str_cstr( cpath );
				char *up = cvt_str_cstr( upath );
				Global_setCMapsPath( cp, up );
				free( cp );
				free( up );
			}
			/// <summary>
			/// Set CMYK resource path and load it.
			/// </summary>
			/// <param name="path">Path of CMYK resource</param>
			/// <returns>True if successed otherwise false</returns>
			static Boolean SetCMYKICC(String ^path)
			{
				char *cp = cvt_str_cstr(path);
				bool ret = Global_setCMYKICC(cp);
				free(cp);
				return ret;
			}
			/// <summary>
			/// Start to load font files. All font files will be loaded into a font list.
			/// </summary>
			static void FontFileListStart()
			{
				Global_fontfileListStart();
			}
			/// <summary>
			/// Add specified font file into font list
			/// </summary>
			/// <param name="path">Path of the font file</param>
			static void FontFileListAdd( String ^path )
			{
				char *cp = cvt_str_cstr( path );
				Global_fontfileListAdd( cp );
				free( cp );
			}
			/// <summary>
			/// End adding font files to font list
			/// </summary>
			static void FontFileListEnd()
			{
				Global_fontfileListEnd();
			}
			/// <summary>
			/// Map a font with a different font name. If the original font resource with that font name is missing the mapped font will be used as a substitute
			/// </summary>
			/// <param name="map_name">A different font name</param>
			/// <param name="name">The name of the font which will be mapped</param>
			/// <returns>True if successed, otherwise false</returns>
			static bool FontFileMapping(String ^map_name, String ^name)
			{
				char *mname = cvt_str_cstr(map_name);
				char *dname = cvt_str_cstr(name);
				bool ret = Global_fontfileMapping(mname, dname);
				free(mname);
				free(dname);
				return ret;
			}
			/// <summary>
			/// Get available font count
			/// </summary>
			/// <returns>Available font count</returns>
			static int GetFaceCount()
			{
				return Global_getFaceCount();
			}
			/// <summary>
			/// Get font face name at specified index position
			/// </summary>
			/// <param name="index">The index position of the font</param>
			/// <returns>Name of the font face if found</returns>
			static String ^GetFaceName( int index )
			{
				return cvt_cstr_str(Global_getFaceName(index));
			}
			/// <summary>
			/// Set a font as default font for specified character set
			/// </summary>
			/// <param name="collection">Collection name of the character set, e.g. GB1, Japan1, Korea1, etc.</param>
			/// <param name="name">Name of the font which will be set as default font</param>
			/// <param name="fixed">If the font will be used as fixed default font</param>
			/// <returns>True if successed, otherwise false</returns>
			static Boolean SetDefaultFont( String ^collection, String ^name, Boolean fixed )
			{
				char *cc = cvt_str_cstr(collection);
				char *cn = cvt_str_cstr(name);
				bool ret = Global_setDefaultFont( cc, cn, fixed );
				free( cc );
				free( cn );
				return ret;
			}
			/// <summary>
			/// Load standard font
			/// </summary>
			/// <param name="index">Index of the font</param>
			/// <param name="path">Path of the font file</param>
			static void LoadStdFont( int index, String ^path )
			{
				char *cp = cvt_str_cstr( path );
				Global_loadStdFont( index, cp );
				free( cp );
			}
			/// <summary>
			/// Set font for annotations
			/// </summary>
			/// <param name="name">Name of the font</param>
			/// <returns>True if successed, otherwise false</returns>
			static Boolean SetAnnotFont( String ^name )
			{
				char *cn = cvt_str_cstr( name );
				bool ret = Global_setAnnotFont( cn );
				free( cn );
				return ret;
			}
			/// <summary>
			/// Set the transparence property of annotations
			/// </summary>
			/// <param name="color">A color containing the transparence information</param>
			static void SetAnnotTransparence( unsigned int color )
			{
				Global_setAnnotTransparency( color );
			}
			/// <summary>
			/// Activate Radaee core lib with specified serial
			/// </summary>
			/// <param name="serial">The serial to activate Radaee core lib</param>
			/// <returns>
			/// License type activated, possible values are:
			/// <0: failed to activate Radaee lib
			/// 1: standard license actived.
			/// 2: professional license actived.
			/// 3. premium license actived.</returns>
			static int Active(String^ serial)
			{
				return Global_active(serial);
			}
			/// <summary>
			/// Get Radaee core lib version
			/// </summary>
			/// <returns>Core lib version</returns>
			static String^ GetVersion()
			{
				char sver[32];
				wchar_t wsver[32];
				Global_getVersion(sver);
				::MultiByteToWideChar(CP_ACP, 0, sver, -1, wsver, 31);
				return ref new String(wsver);
			}
			/*
			static int Active(String ^company, String ^email, String^ key)
			{
				char scom[1024];
				char smail[1024];
				char skey[1024];
				::WideCharToMultiByte(CP_UTF8, 0, company->Data(), -1, scom, 1020, NULL, NULL);
				::WideCharToMultiByte(CP_UTF8, 0, email->Data(), -1, smail, 1020, NULL, NULL);
				::WideCharToMultiByte(CP_UTF8, 0, key->Data(), -1, skey, 1020, NULL, NULL);
				if (Global_activeStandard(scom, smail, skey)) return 1;
				if (Global_activeProfession(scom, smail, skey)) return 2;
				if (Global_activePremium(scom, smail, skey)) return 3;
				return 0;
			}
			*/
			
			/// <summary>
			/// Get current zoom level
			/// </summary>
			static property float ZoomLevel
			{
				float get(){ return zoom_level; }
				void set(float level){ zoom_level = level; }
			}
			/// <summary>
			/// Draw icon to a Bitmap object
			/// </summary>
			/// <param name="atype">1(text note) or 17(file attachment)</param>
			/// <param name="icon">Index of the icon in icon type list</param>
			/// <param name="bmp">Image resource of the icon</param>
			/// <returns>True if success, otherwise false</returns>
			static bool drawAnnotIcon(int atype, int icon, WriteableBitmap^ bmp)
			{
				return Global_drawAnnotIcon(atype, icon, bmp);
			}
			/// <summary>
			/// Draw a dash line with specified image resource
			/// </summary>
			/// <param name="dash">Dash line data</param>
			/// <param name="bmp">Image resource which will be used to draw the dash line</param>
			static void drawDashLine(const Array<float> ^dash, WriteableBitmap^ bmp)
			{
				if(dash && dash->Length > 0)
					Global_drawDashLine(dash->Data, dash->Length, bmp);
				else
					Global_drawDashLine(NULL, 0, bmp);
			}
			/// <summary>
			/// Draw a bitmap image as start/end point of line annotations
			/// </summary>
			/// <param name="head"></param>
			/// <param name="bmp"></param>
			static void drawLineHead(int head, WriteableBitmap^ bmp)
			{
				Global_drawLineHead(head, bmp);
			}
		private:
			static float zoom_level;
		};
		public ref class PDFPath sealed
		{
		public:
			PDFPath()
			{
				m_path = Path_create();
			}
			/// <summary>
			/// Perform a move to operation, update current postion
			/// </summary>
			/// <param name="x">x coordinate of the postion to move to</param>
			/// <param name="y">y coordinate of the postion to move to</param>
			void MoveTo( float x, float y )
			{
				Path_moveTo( m_path, x, y );
			}
			/// <summary>
			/// Perform a line to operation from current postiion to specified postion
			/// </summary>
			/// <param name="x">x coordinate of the end postion of the line</param>
			/// <param name="y">y coordinate of the end postion of the line</param>
			void LineTo( float x, float y )
			{
				Path_lineTo( m_path, x, y );
			}
			/// <summary>
			/// Draw a curve from start postiion to specified postion
			/// </summary>
			/// <param name="x1">x coordinate of the start postion of the curve</param>
			/// <param name="y1">y coordinate of the start postion of the curve</param>
			/// <param name="x2">x coordinate of the end postion of the curve</param>
			/// <param name="y2">y coordinate of the end postion of the curve</param>
			/// <param name="x3">x coordinate of the peak postion of the curve</param>
			/// <param name="y4">y coordinate of the peak postion of the curve</param>
			void CurveTo( float x1, float y1, float x2, float y2, float x3, float y3 )
			{
				Path_curveTo( m_path, x1, y1, x2, y2, x3, y3 );
			}
			/// <summary>
			/// Perform a line to operation from current position to the start position of the path to close it.
			/// </summary>
			void Close()
			{
				Path_closePath( m_path );
			}
			/// <summary>
			/// Get node count of the path
			/// </summary>
			property int NodesCnt
			{
				int get(){return Path_getNodeCount(m_path);}
			}
			/// <summary>
			/// Get the operation on specified node
			/// </summary>
			/// <param name="index">Index of node</param>
			/// <returns>Operation on the node</returns>
			int GetOP( int index )
			{
				PDF_POINT pt;
				return Path_getNode( m_path, index, &pt );
			}
			/// <summary>
			/// Get specified point
			/// </summary>
			/// <param name="index">Index of the point</param>
			/// <returns>A PDFPoint object</returns>
			PDFPoint GetPoint( int index )
			{
				PDF_POINT pt;
				Path_getNode( m_path, index, &pt );
				return *(PDFPoint *)&pt;
			}
		private:
			PDFPath(PDF_PATH path)
			{
				m_path = path;
			}
			friend PDFMatrix;
			friend PDFPageContent;
			friend PDFPage;
			friend PDFAnnot;
			~PDFPath()
			{
				Path_destroy( m_path );
			}
			PDF_PATH m_path;
		};
		public ref class PDFInk sealed
		{
		public:
			PDFInk( float width, unsigned int color )
			{
				m_ink = Ink_create( width, color );
			}
			/// <summary>
			/// Trigger a down event for ink
			/// </summary>
			/// <param name="x">x coordinate of the postion of down event</param>
			/// <param name="y">y coordinate of the postion of down event</param>
			void Down( float x, float y )
			{
				Ink_onDown( m_ink, x, y );
			}
			/// <summary>
			/// Trigger a move event for ink
			/// </summary>
			/// <param name="x">x coordinate of the postion of move event</param>
			/// <param name="y">y coordinate of the postion of move event</param>
			void Move( float x, float y )
			{
				Ink_onMove( m_ink, x, y );
			}
			/// <summary>
			/// Trigger a up event for ink
			/// </summary>
			/// <param name="x">x coordinate of the postion of up event</param>
			/// <param name="y">y coordinate of the postion of up event</param>
			void Up( float x, float y )
			{
				Ink_onUp( m_ink, x, y );
			}
			/// <summary>
			/// Get node count in the ink
			/// </summary>
			property int NodesCnt
			{
				int get(){return Ink_getNodeCount(m_ink);}
			}
			/// <summary>
			/// Get operation on specified node
			/// </summary>
			/// <param name="index">Index of node</param>
			/// <returns>The operation</returns>
			int GetOP( int index )
			{
				PDF_POINT pt;
				return Ink_getNode( m_ink, index, &pt );
			}
			/// <summary>
			/// Get point object at with index
			/// </summary>
			/// <param name="index">Index of the point</param>
			/// <returns>A PDFPoint object</returns>
			PDFPoint GetPoint( int index )
			{
				PDF_POINT pt;
				Ink_getNode( m_ink, index, &pt );
				return *(PDFPoint *)&pt;
			}
		private:
			friend PDFMatrix;
			friend PDFPage;
			~PDFInk()
			{
				Ink_destroy( m_ink );
			}
			PDF_INK m_ink;
		};
		public ref class PDFMatrix sealed
		{
		public:
			PDFMatrix( float scalex, float scaley, float x0, float y0  )
			{
				m_mat = Matrix_createScale( scalex, scaley, x0, y0 );
			}
			PDFMatrix( float xx, float yx, float xy, float yy, float x0, float y0  )
			{
				m_mat = Matrix_create( xx, yx, xy, yy, x0, y0 );
			}
			void Invert()
			{
				Matrix_invert( m_mat );
			}
			/// <summary>
			/// Transform a PDFPath object
			/// </summary>
			/// <param name="path">The PDFPath object to transform</param>
			void TransformPath( PDFPath ^path )
			{
				Matrix_transformPath( m_mat, path->m_path );
			}
			/// <summary>
			/// Transform a PDFInk object
			/// </summary>
			/// <param name="path">The PDFInk object to transform</param>
			void TransformInk( PDFInk ^ink )
			{
				Matrix_transformInk( m_mat, ink->m_ink );
			}
			/// <summary>
			/// Transform a rectangle
			/// </summary>
			/// <param name="path">The PDFRect object to transform</param>
			PDFRect TransformRect( PDFRect rect )
			{
				Matrix_transformRect( m_mat, (PDF_RECT *)&rect );
				return rect;
			}
			/// <summary>
			/// Transform a point
			/// </summary>
			/// <param name="path">The PDFPoint object to transform</param>
			PDFPoint TransformPoint( PDFPoint point )
			{
				Matrix_transformPoint( m_mat, (PDF_POINT *)&point );
				return point;
			}
		private:
			PDFMatrix()
			{
				m_mat = NULL;
			}
			friend PDFPage;
			friend PDFPageContent;
			~PDFMatrix()
			{
				Matrix_destroy( m_mat );
			}
			PDF_MATRIX m_mat;
		};
		public ref class PDFDIB sealed
		{
		public:
			PDFDIB(int w, int h )
			{
				m_dib = Global_dibGet(NULL, w, h);
				//SoftwareBitmapSource^ source = ref new SoftwareBitmapSource();
				//source->SetBitmapAsync(m_bmp);
			}
			/// <summary>
			/// Resize current PDFDIB object
			/// </summary>
			/// <param name="w">New width of the PDFDIB</param>
			/// <param name="h">New height of the PDFDIb</param>
			void Resize(int w, int h)
			{
				m_dib = Global_dibGet( m_dib, w, h );
			}
			/// <summary>
			/// Save PDFDIB object to a local JPEG file
			/// </summary>
			/// <param name="path">Path of output file</param>
			/// <param name="quality">Render quality of output file</param>
			/// <returns>True if sccessed, otherwise false</returns>
			Boolean SaveJPG(String ^path, int quality)
			{
				const wchar_t *wtxt = path->Data();
				char tmp[512];
				::WideCharToMultiByte(CP_ACP, 0, wtxt, -1, tmp, 512, NULL, NULL);
				return Global_dibSaveJPG(m_dib, tmp, quality);
			}
			/// <summary>
			/// Reset the PDFDIB content with specified color
			/// </summary>
			/// <param name="color">Color to reset the PDFDIB</param>
			void Reset(unsigned int color)
			{
				int w = Global_dibGetWidth(m_dib);
				int h = Global_dibGetHeight(m_dib);
				unsigned int* dat_cur = (unsigned int *)Global_dibGetData(m_dib);
				unsigned int* dat_end = dat_cur + (w * h);
				while (dat_cur < dat_end) *dat_cur++ = color;
			}
			/// <summary>
			/// Draw content from another PDFDIB object
			/// </summary>
			/// <param name="src">The PDFDIB object holding the content</param>
			/// <param name="x">x coordinate of the position to draw</param>
			/// <param name="y">y coordinate of the position to draw</param>
			void DrawDIB(PDFDIB^ src, int x, int y)
			{
				int sx = 0;
				int sy = 0;
				int dx = x;
				int dy = y;
				int dw = Global_dibGetWidth(m_dib);
				int dh = Global_dibGetHeight(m_dib);
				int sw = Global_dibGetWidth(src->m_dib);
				int sh = Global_dibGetHeight(src->m_dib);
				if (dx < 0)
				{
					sx -= dx;
					dx = 0;
				}
				if (dy < 0)
				{
					sy -= dy;
					dy = 0;
				}
				int w = dw - dx;
				int h = dh - dy;
				int w1 = sw - sx;
				int h1 = sh - sy;
				if (w < w1) w = w1;
				if (h < h1) h = h1;

				BYTE *psrc = (BYTE *)Global_dibGetData(src->m_dib) + ((sy * sw + sx) << 2);
				BYTE *pdst = (BYTE *)Global_dibGetData(m_dib) + ((dy * dw + dx) << 2);
				while (h > 0)
				{
					cpy_clr((unsigned int *)pdst, (const unsigned int*)psrc, w);
					pdst += (dw << 2);
					psrc += (sw << 2);
					h--;
				}
			}
			/// <summary>
			/// Get width of the PDFDIB
			/// </summary>
			property int Width
			{
				int get() {return Global_dibGetWidth( m_dib );}
			}
			/// <summary>
			/// Get height of the PDFDIB
			/// </summary>
			property int Height
			{
				int get() {return Global_dibGetHeight( m_dib );}
			}
			/// <summary>
			/// Get content data of the PDFDIB
			/// </summary>
			property Array<BYTE> ^Data
			{
				Array<BYTE> ^get()
				{
					int w = Global_dibGetWidth( m_dib );
					int h = Global_dibGetHeight( m_dib );
					BYTE *dat = (BYTE *)Global_dibGetData(m_dib);
					return ArrayReference<BYTE>(dat, w * h * 4 );
				}
			}
		private:
			~PDFDIB()
			{
				Global_dibFree( m_dib );
				m_dib = NULL;
			}
			ID2D1Bitmap1 *genDXBmp(ID2D1DeviceContext *ctx)
			{
				D2D1_SIZE_U size;
				size.width = Global_dibGetWidth(m_dib);
				size.height = Global_dibGetHeight(m_dib);
				BYTE *dat = (BYTE *)Global_dibGetData(m_dib);
				D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_NONE,
					D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
					96, 96);
				ID2D1Bitmap1 *bmp = NULL;
				ctx->CreateBitmap(size, dat, size.width << 2, &bitmapProperties, &bmp);
				return bmp;
			}
			static inline void cpy_clr(unsigned int* dst, const unsigned int* src, int count)
			{
				unsigned int* dst_end = dst + count;
				while (dst < dst_end) *dst++ = *src++;
			}
			friend PDFBmp;
			friend PDFPage;
			friend PDFAnnot;
			//friend class RDPDFLib::view::DXBlock;
			PDF_DIB m_dib;
		};
		public ref class PDFSoftBmp sealed
		{
		public:
			PDFSoftBmp(int w, int h)
			{
				m_w = w;
				m_h = h;
				m_dib = ref new SoftwareBitmap(BitmapPixelFormat::Bgra8, w, h, BitmapAlphaMode::Premultiplied);
				m_bmp = Global_lockSoftBitmap(m_dib);
			}
			/// <summary>
			/// Get width of the bitmap
			/// </summary>
			property int Width
			{
				int get() { return m_w; }
			}
			/// <summary>
			/// Get height of the bitmap
			/// </summary>
			property int Height
			{
				int get() { return m_h; }
			}
			/// <summary>
			/// Get bitmap of the bitmap as SoftwareBitmap
			/// </summary>
			property SoftwareBitmap^ Data
			{
				SoftwareBitmap^ get()
				{
					return m_dib;
				}
			}
			/// <summary>
			/// Reset the PDFSoftBmp content with specified color
			/// </summary>
			/// <param name="color">Color to reset the PDFSoftBmp</param>
			void Reset(unsigned int color)
			{
				Global_eraseColor(m_bmp, color);
			}
			/// <summary>
			/// Save PDFSoftBmp object to a local JPEG file
			/// </summary>
			/// <param name="path">Path of output file</param>
			/// <param name="quality">Render quality of output file</param>
			/// <returns>True if sccessed, otherwise false</returns>
			Boolean SaveJPG(String^ path, int quality)
			{
				const wchar_t* wtxt = path->Data();
				char tmp[512];
				::WideCharToMultiByte(CP_ACP, 0, wtxt, -1, tmp, 512, NULL, NULL);
				return Global_saveBitmapJPG(m_bmp, tmp, quality);
			}
		private:
			~PDFSoftBmp()
			{
				Global_unlockBitmap(m_bmp);
				m_dib = nullptr;
			}
			friend PDFPage;
			friend PDFAnnot;
			SoftwareBitmap^ m_dib;
			PDF_BMP m_bmp;
			int m_w;
			int m_h;
		};

		//extern byte g_tiny_bmp[];
		public ref class PDFBmp sealed
		{
		public:
			PDFBmp(int w, int h)
			{
				m_w = w;
				m_h = h;
				m_dib = ref new WriteableBitmap(w, h);
				m_bmp = Global_lockBitmap( m_dib );
			}
			/// <summary>
			/// Get width of the bitmap
			/// </summary>
			property int Width
			{
				int get() { return m_w; }
			}
			/// <summary>
			/// Get height of the bitmap
			/// </summary>
			property int Height
			{
				int get() { return m_h; }
			}
			/// <summary>
			/// Get bitmap of the bitmap as WriteableBitmap
			/// </summary>
			property WriteableBitmap ^Data
			{
				WriteableBitmap ^get()
				{
					return m_dib;
				}
			}
			/// <summary>
			/// Reset the PDFSoftBmp content with specified color
			/// </summary>
			/// <param name="color">Color to reset the PDFSoftBmp</param>
			void Reset( unsigned int color )
			{
				Global_eraseColor( m_bmp, color );
			}
			/// <summary>
			/// Save PDFBmp object to a local JPEG file
			/// </summary>
			/// <param name="path">Path of output file</param>
			/// <param name="quality">Render quality of output file</param>
			/// <returns>True if sccessed, otherwise false</returns>
			Boolean SaveJPG(String ^path, int quality)
			{
				const wchar_t *wtxt = path->Data();
				char tmp[512];
				::WideCharToMultiByte(CP_ACP, 0, wtxt, -1, tmp, 512, NULL, NULL);
				return Global_saveBitmapJPG(m_bmp, tmp, quality);
			}
			/// <summary>
			/// Detach the content to a WriteableBitmap object
			/// </summary>
			/// <returns>The WriteableBitmap object</returns>
			WriteableBitmap^ Detach()
			{
				if (m_bmp)
				{
					Global_unlockBitmap(m_bmp);
					m_bmp = NULL;
					WriteableBitmap^ ret = m_dib;
					m_dib = nullptr;
					return ret;
				}
				else return nullptr;
			}
			/// <summary>
			/// Draw content from a PDFDIB object to PDFBmp
			/// </summary>
			/// <param name="src">PDFDIB holding the content</param>
			/// <param name="x">x coordinate of the position to draw</param>
			/// <param name="y">y coordinate of the position to draw</param>
			void DrawDIB(PDFDIB^ src, int x, int y)
			{
				if (src) Global_drawDIB(m_bmp, src->m_dib, x, y);
			}
			virtual ~PDFBmp()
			{
				if (m_bmp)
				{
					Global_unlockBitmap(m_bmp);
					m_bmp = NULL;
					m_dib = nullptr;
				}
			}
		internal:
			friend PDFPage;
			friend PDFAnnot;
			WriteableBitmap^m_dib;
			PDF_BMP m_bmp;
			int m_w;
			int m_h;
		};
		public interface class PDFStream
		{
		public:
			/// <summary>
			/// Get if the stream writeable
			/// </summary>
			/// <returns>True or false</returns>
			bool Writeable();
			/// <summary>
			/// Get available data length
			/// </summary>
			/// <returns>Length of the data</returns>
			long long GetLength();
			/// <summary>
			/// Get current position in the stream
			/// </summary>
			/// <returns>Index of the position</returns>
			long long GetPosition();
			/// <summary>
			/// Set and jump to specified postion in the stream
			/// </summary>
			/// <param name="pos">Position in the stream</param>
			/// <returns>True if successed, otherwise false</returns>
			bool SetPosition(long long pos);
			/// <summary>
			/// Read data from stream into buffer
			/// </summary>
			/// <param name="buf">buffer to receive the data</param>
			/// <returns>Length of bytes read</returns>
			int Read( WriteOnlyArray<BYTE> ^buf );
			/// <summary>
			/// Write data into stream from buffer
			/// </summary>
			/// <param name="buf">Buffer holding the data</param>
			/// <returns>Length of bytes written</returns>
			int Write( const Array<BYTE> ^buf );
			/// <summary>
			/// Close the stream
			/// </summary>
			void Close();
			/// <summary>
			/// Flush changes into stream
			/// </summary>
			void Flush();
		};
		public interface class PDFJSDelegate
		{
		public:
			/// <summary>
			/// Console command
			/// </summary>
			/// <param name="cmd">
			/// *0: clear console.
			/// *1: hide console.
			///	*2: print line on sonsole.
			///	*3: show console.</param>
			/// <param name="para">Only valid when cmd == 2;</param>
			void OnConsole(int cmd, String ^para);
			/// <summary>
			/// Show a alert dialog on screen.
			/// </summary>
			/// <param name="btn">uttons show on dialog.
			/// *0: OK
			///	*1: OK, Cancel
			///	*2: Yes, No
			///	*3: Yes, No, Cancel</param>
			/// <param name="msg">Message to be show.</param>
			/// <param name="title">Title to be show.</param>
			/// <returns>Tbutton user clicked. Values as below:
			/// *1: OK
			///	*2: Cancel
			///	*3: No
			///	*4: Yes</returns>
			int OnAlert(int btn, String ^msg, String ^title);
			/// <summary>
			/// Callback when document closed.
			/// </summary>
			/// <returns>True is the Document object need save, otherwise false.</returns>
			bool OnDocClose();
			/// <summary>
			/// Generate a tmp file name that JS needed in background.
			/// </summary>
			/// <returns>Absolute path to temp path generated.</returns>
			String ^OnTmpFile();
			/// <summary>
			/// Callback when an Uncaught exception appears.
			/// </summary>
			/// <param name="code">Error code.</param>
			/// <param name="msg">Error message.</param>
			void OnUncaughtException(int code, String ^msg);
		};
		public ref class PDFObj sealed
		{
		public:
			PDFObj()
			{
				m_obj = NULL;
			}
			property int type
			{
				int get(){ return Obj_getType(m_obj); }
			}
			property int IntVal
			{
				int get(){ return Obj_getInt(m_obj); }
				void set(int v){ Obj_setInt(m_obj, v); }
			}
			property float RealVal
			{
				float get(){ return Obj_getReal(m_obj); }
				void set(float v){ Obj_setReal(m_obj, v); }
			}
			property bool BoolVal
			{
				bool get(){ return Obj_getBoolean(m_obj); }
				void set(bool v){ Obj_setBoolean(m_obj, v); }
			}
			property String ^NameVal
			{
				String ^get()
				{
					const char *cname = Obj_getName(m_obj);
					if (!cname) return nullptr;
					int clen = strlen(cname);
					wchar_t *wsname = (wchar_t *)malloc(sizeof(wchar_t) * (clen + 1));
					MultiByteToWideChar(CP_ACP, 0, cname, -1, wsname, clen + 1);
					String ^ret = ref new String(wsname);
					free(wsname);
					return ret;
				}
				void set(String ^name)
				{
					const wchar_t *wsname = name->Data();
					int wlen = name->Length();
					char *cname = (char *)malloc(wlen * 4 + 4);
					WideCharToMultiByte(CP_ACP, 0, wsname, -1, cname, wlen * 4 + 4, NULL, NULL);
					Obj_setName(m_obj, cname);
					free(cname);
				}
			}
			property String ^AsciiStringVal
			{
				String ^get()
				{
					const char *cname = Obj_getAsciiString(m_obj);
					if (!cname) return nullptr;
					int clen = strlen(cname);
					wchar_t *wsname = (wchar_t *)malloc(sizeof(wchar_t) * (clen + 1));
					MultiByteToWideChar(CP_ACP, 0, cname, -1, wsname, clen + 1);
					String ^ret = ref new String(wsname);
					free(wsname);
					return ret;
				}
				void set(String ^name)
				{
					const wchar_t *wsname = name->Data();
					int wlen = name->Length();
					char *cname = (char *)malloc(wlen * 4 + 4);
					WideCharToMultiByte(CP_ACP, 0, wsname, -1, cname, wlen * 4 + 4, NULL, NULL);
					Obj_setAsciiString(m_obj, cname);
					free(cname);
				}
			}
			property String ^TextStringVal
			{
				String ^get()
				{
					return Obj_getTextString(m_obj);
				}
				void set(String ^name)
				{
					Obj_setTextString(m_obj, name->Data());
				}
			}
			property Array<BYTE> ^HexStringVal
			{
				Array<BYTE> ^get()
				{
					int len;
					unsigned char *data = Obj_getHexString(m_obj, &len);
					if (!data) return nullptr;
					return ArrayReference<BYTE>((BYTE *)data, len);
				}
				void set(const Array<BYTE> ^v)
				{
					BYTE *data = v->Data;
					int len = v->Length;
					Obj_setHexString(m_obj, data, len);
				}
			}
			property PDFRef RefVal
			{
				PDFRef get()
				{
					PDFRef ref;
					ref.ref = Obj_getReference(m_obj);
					return ref;
				}
				void set(PDFRef ref)
				{
					Obj_setReference(m_obj, ref.ref);
				}
			}
			/// <summary>
			/// Set a PDF_OBJ as dictionary
			/// </summary>
			void SetDictionary()
			{
				Obj_dictGetItemCount(m_obj);
			}

			/// <summary>
			/// Get dictionary item count
			/// </summary>
			/// <returns>Item count</returns>
			int DictGetItemCount()
			{
				return Obj_dictGetItemCount(m_obj);
			}
			/// <summary>
			/// Get tag of a dictionary item
			/// </summary>
			/// <param name="index">Index of the item</param>
			/// <returns>The tag content</returns>
			String ^DictGetItemTag(int index)
			{
				const char *tag = Obj_dictGetItemName(m_obj, index);
				if (!tag) return nullptr;
				int clen = strlen(tag);
				wchar_t *wsname = (wchar_t *)malloc(sizeof(wchar_t) * (clen + 1));
				MultiByteToWideChar(CP_ACP, 0, tag, -1, wsname, clen + 1);
				String ^ret = ref new String(wsname);
				free(wsname);
				return ret;
			}
			/// <summary>
			/// Get a dictionary item with index
			/// </summary>
			/// <param name="index">Index of the item</param>
			/// <returns>A PDFObj object which is the item</returns>
			PDFObj ^DictGetItem(int index)
			{
				PDF_OBJ obj = Obj_dictGetItemByIndex(m_obj, index);
				if (!obj) return nullptr;
				PDFObj ^ret = ref new PDFObj();
				ret->m_obj = obj;
				return ret;
			}
			/// <summary>
			/// Get a dictionary item with tag
			/// </summary>
			/// <param name="tag">Tag of the item</param>
			/// <returns>A PDFObj object which is the item</returns>
			PDFObj ^DictGetItem(String ^tag)
			{
				const wchar_t *wsname = tag->Data();
				int wlen = tag->Length();
				char *cname = (char *)malloc(wlen * 4 + 4);
				WideCharToMultiByte(CP_ACP, 0, wsname, -1, cname, wlen * 4 + 4, NULL, NULL);
				PDF_OBJ obj = Obj_dictGetItemByName(m_obj, cname);
				free(cname);
				if (!obj) return nullptr;
				PDFObj ^ret = ref new PDFObj();
				ret->m_obj = obj;
				return ret;
			}
			/// <summary>
			/// Set a dictionary item with tag
			/// </summary>
			/// <param name="tag">Tag of the item</param>
			void DictSetItem(String ^tag)
			{
				const wchar_t *wsname = tag->Data();
				int wlen = tag->Length();
				char *cname = (char *)malloc(wlen * 4 + 4);
				WideCharToMultiByte(CP_ACP, 0, wsname, -1, cname, wlen * 4 + 4, NULL, NULL);
				Obj_dictSetItem(m_obj, cname);
				free(cname);
			}
			/// <summary>
			/// Remove a dictionary item with tag
			/// </summary>
			/// <param name="tag">Tag of the item</param>
			void DictRemoveItem(String ^tag)
			{
				const wchar_t *wsname = tag->Data();
				int wlen = tag->Length();
				char *cname = (char *)malloc(wlen * 4 + 4);
				WideCharToMultiByte(CP_ACP, 0, wsname, -1, cname, wlen * 4 + 4, NULL, NULL);
				Obj_dictRemoveItem(m_obj, cname);
				free(cname);
			}
			/// <summary>
			/// Set a PDF_OBJ object as an array
			/// </summary>
			void SetArray()
			{
				Obj_arrayClear(m_obj);
			}
			/// <summary>
			/// Get item count of an array
			/// </summary>
			/// <returns>The number of item count</returns>
			int ArrayGetItemCount()
			{
				return Obj_arrayGetItemCount(m_obj);
			}
			/// <summary>
			/// Get an item from the array with index
			/// </summary>
			/// <param name="index">Index of the item</param>
			/// <returns>The item with the index if found</returns>
			PDFObj ^ArrayGetItem(int index)
			{
				PDF_OBJ obj = Obj_arrayGetItem(m_obj, index);
				if (!obj) return nullptr;
				PDFObj ^ret = ref new PDFObj();
				ret->m_obj = obj;
				return ret;
			}
			/// <summary>
			/// Append a new item to existing array
			/// </summary>
			void ArrayAppendItem()
			{
				Obj_arrayAppendItem(m_obj);
			}
			/// <summary>
			/// Insert a new item to existing array with specified position
			/// </summary>
			/// <param name="index">The position to insert the item</param>
			void ArrayInsertItem(int index)
			{
				Obj_arrayInsertItem(m_obj, index);
			}
			/// <summary>
			/// Remove an item from existing array with specified position
			/// </summary>
			/// <param name="index">The position of the item to remove</param>
			void ArrayRemoveItem(int index)
			{
				Obj_arrayRemoveItem(m_obj, index);
			}
			/// <summary>
			/// Clear all elements in existing array
			/// </summary>
			void ArrayClear()
			{
				Obj_arrayClear(m_obj);
			}
		private:
			friend ref class PDFDoc;
			PDF_OBJ m_obj;
		};
		public ref class PDFDoc sealed
		{
		public:
			PDFDoc();
			static void SetOpenFlag(int flag)
			{
				Document_setOpenFlag(flag);
			}
			PDF_ERROR Open( IRandomAccessStream ^stream, String ^password );
			PDF_ERROR OpenStream( PDFStream ^stream, String ^password );
			PDF_ERROR OpenPath( String ^path, String ^password );
			/// <summary>
			/// Create a new PDF document with output file stream
			/// </summary>
			/// <param name="stream">A IRandomAccessStream which is typically generated from a opened file. Represents the output file object.</param>
			/// <returns>Result for creating the document. Refer to definition of PDF_ERROR</returns>
			PDF_ERROR Create( IRandomAccessStream ^stream )
			{
				PDF_ERR err;
				m_doc = Document_create( stream, &err );
				return (PDF_ERROR)err;
			}
			/// <summary>
			/// Create a new PDF document with output stream
			/// </summary>
			/// <param name="stream">A PDFStream object to accessing the new document data</param>
			/// <returns>Result for creating the document. Refer to definition of PDF_ERROR</returns>
			PDF_ERROR CreateStream( PDFStream ^stream )
			{
				PDF_ERR err;
				m_inner = new PDFStreamInner;
				m_inner->Open( stream );
				m_doc = Document_createForStream( m_inner, &err );
				return (PDF_ERROR)err;
			}
			/// <summary>
			/// Create a new PDF document with output file
			/// </summary>
			/// <param name="path">Path of output file</param>
			/// <returns>Result for creating the document. Refer to definition of PDF_ERROR</returns>
			PDF_ERROR CreatePath( String ^path )
			{
				PDF_ERR err;
				char *cpath = cvt_str_cstr( path );
				m_doc = Document_createForPath( cpath, &err );
				free( cpath );
				return (PDF_ERROR)err;
			}
			property int LinearizedStatus
			{
				int get() { return Document_getLinearizedStatus(m_doc); }
			}
			/// <summary>
			/// Set a cache file
			/// </summary>
			/// <param name="path">path of the cache file</param>
			void SetCahce( String ^path )
			{
				char *cpath = cvt_str_cstr( path );
				Document_setCache( m_doc, cpath );
				free( cpath );
			}
			/// <summary>
			/// Execute embedded JS
			/// </summary>
			/// <param name="js">The JS code to execute</param>
			/// <param name="del">A callback for the execution</param>
			/// <returns>True if the code is successfully executed, otherwise false</returns>
			bool RunJS(String ^js, PDFJSDelegate ^del)
			{
				PDFJSDelegateInner idel(del);
				const wchar_t *wstmp = js->Data();
				int len = wcslen(wstmp) + 1;
				char *stmp = (char *)malloc(sizeof(wchar_t) * len);
				::WideCharToMultiByte(CP_ACP, 0, wstmp, -1, stmp, len * sizeof(wchar_t), NULL, NULL);
				bool ret = Document_runJS(m_doc, stmp, &idel);
				free(stmp);
				return ret;
			}
			/// <summary>
			/// Save changes to document
			/// </summary>
			/// <returns>True if successed, otherwise false</returns>
			Boolean Save();
			/// <summary>
			/// Save the document as a new file
			/// </summary>
			/// <param name="path">The path of output file</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean SaveAs(String ^path)
			{
				if (!path) return false;
				return Document_saveAsW(m_doc, path->Data());
			}
			/// <summary>
			/// Encrypt the document with passwords and save it as a new file
			/// </summary>
			/// <param name="dst">Path to save�� same as path parameter of SaveAs.</param>
			/// <param name="User_pswd">User password, can be null.</param>
			/// <param name="Owner_pswd">Owner password, can be null.</param>
			/// <param name="perm">permission to set, same as GetPermission() method.
			/// *bit 1 - 2 reserved
			///	*bit 3(0x4) print
			///	*bit 4(0x8) modify
			///	*bit 5(0x10) extract text or image
			///	*others: see PDF reference</param>
			/// <param name="method">Set 3 means using AES 256bits encrypt(Acrobat X), V=5 and R = 6 mode, others AES with V=4 and R=4 mode.</param>
			/// <param name="fid">Must be 32 bytes for file ID. it is divided to 2 array in native library, as each 16 bytes.</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean EncryptAs(String^ dst, String ^User_pswd, String ^Owner_pswd, int perm, int method, const Array<BYTE> ^fid)
			{
				if (!dst || !fid || fid->Length < 32) return false;
				const wchar_t* ws_path = dst->Data();
				const wchar_t* ws_upswd = User_pswd->Data();
				const wchar_t* ws_opswd = Owner_pswd->Data();
				const BYTE* pb_fid = fid->Data;
				return Document_encryptAsW(m_doc, ws_path, ws_upswd, ws_opswd, perm, method, pb_fid);
			}
			/// <summary>
			/// Close opened document
			/// </summary>
			void Close();
			/// <summary>
			/// Get the max page width and height from all pages
			/// </summary>
			property PDFPoint MaxPageSize
			{
				PDFPoint get()
				{
					PDF_POINT pt = Document_getPagesMaxSize(m_doc);
					return *(PDFPoint *)&pt;
				}
			}
			float GetPageWidth(int pageno);
			float GetPageHeight(int pageno);
			/// <summary>
			/// Get label of specified page
			/// </summary>
			/// <param name="pageno">The index unmber of target page</param>
			/// <returns>The label of page</returns>
			String ^GetPageLabel(int pageno)
			{
				wchar_t wtxt[512];
				if (!Document_getPageLabel(m_doc, pageno, wtxt, 511)) return nullptr;
				return ref new String(wtxt);
			}
			/// <summary>
			/// Rotate a page with specified angle
			/// </summary>
			/// <param name="pageno">Index number of the target page</param>
			/// <param name="degree">Angle for rotation</param>
			/// <returns>True if successed, otherwise false</returns>
			bool SetPageRotate(int pageno, int degree)
			{
				return Document_setPageRotate(m_doc, pageno, degree);
			}
			String ^GetMeta(String ^tag);
			bool SetMeta(String^ tag, String^ val);
			String ^ExportForm();
			PDFOutline ^GetRootOutline();
			Boolean AddRootOutline( String ^label, int dest, float y );
			PDFPage ^GetPage(int pageno);
			PDFDocImage ^NewImage(WriteableBitmap ^bitmap, bool has_alpha);
			PDFDocImage^ NewImage(SoftwareBitmap^ bitmap, bool has_alpha);
			PDFDocImage^ NewImage(WriteableBitmap^ bitmap, unsigned int matte);
			PDFDocImage^ NewImage(SoftwareBitmap^ bitmap, unsigned int matte);
			PDFDocImage ^NewImageJPEG( String ^path );
			PDFDocImage ^NewImageJPX( String ^path );
			PDFDocFont ^NewFontCID( String ^name, int style );
			PDFDocGState ^NewGState();
			PDFDocForm	^NewForm();
			PDFPage ^NewPage( int pageno, float w, float h );
			Boolean RemovePage( int pageno );
			Boolean MovePage( int srcno, int dstno );
			PDFImportCtx ^ImportStart(PDFDoc ^src);
			property unsigned long long Handler
			{
				unsigned long long get() { return (unsigned long long)m_doc; }
			}
			/// <summary>
			/// Get page count of the document
			/// </summary>
			property int PageCount
			{
				int get() {return Document_getPageCount(m_doc);}
			}
			/// <summary>
			/// Get permission of PDF, this value defined in PDF reference 1.7. Mostly, it means the permission from encryption. This method require a professional or premium license.
			///	bit 1 - 2 reserved
			///	bit 3(0x4) print
			///	bit 4(0x8) modify
			///	bit 5(0x10) extract text or image
			///	others: see PDF reference
			/// </summary>
			property int Permission
			{
				int get() {return Document_getPermission( m_doc );}
			}
			/// <summary>
			/// Get/Set XMP string from document.
			/// </summary>
			property String ^XMP
			{
				String ^get() { return Document_getXMP(m_doc); }
				void set(String^ xmp) { Document_setXMP(m_doc, xmp); }
			}
			/// <summary>
			/// Get get permission of PDF, this value defined in "Perm" entry in Catalog object. Mostly, it means the permission from signature. This method require a professional or premium license.
			/// return 0 means not defined;
			/// 1 means can't modify;
			/// 2 means can modify some form fields;
			/// 3 means can do any modify
			/// </summary>
			property int Perm
			{
				int get() {return Document_getPerm( m_doc );}
			}
			/// <summary>
			/// Get embed files count, for document level. This method require premium license, it always return 0 if using other license type.
			/// </summary>
			property int EFCount
			{
				int get() { return Document_getEFCount(m_doc); }
			}
			/// <summary>
			/// Get embed file description
			/// </summary>
			/// <param name="index">Index of embedded file</param>
			/// <returns>Description of the file if found</returns>
			String^ GetEFDesc(int index)
			{
				return Document_getEFDesc(m_doc, index);
			}
			/// <summary>
			/// Get embed file name
			/// </summary>
			/// <param name="index">Index of embedded file</param>
			/// <returns>Name of the file if found</returns>
			String^ GetEFName(int index)
			{
				return Document_getEFName(m_doc, index);
			}
			/// <summary>
			/// Get embed file data and save it to an output file
			/// </summary>
			/// <param name="index">Index of embedded file</param>
			/// <param name="path">Path of the output file to store embedded file data</param>
			/// <returns>True if successed, otherwise false</returns>
			bool GetEFData(int index, String ^path)
			{
				return Document_getEFData(m_doc, index, path);
			}
			/// <summary>
			/// Get embedded JS count
			/// </summary>
			property int JSCount
			{
				int get() { return Document_getJSCount(m_doc); }
			}
			/// <summary>
			/// Get embedded JS by index
			/// </summary>
			/// <param name="index">Index of the JS</param>
			/// <returns>The content of JS</returns>
			String^ GetJS(int index)
			{
				return Document_getJS(m_doc, index);
			}
			/// <summary>
			/// Get name of embedded JS by index
			/// </summary>
			/// <param name="index">Index of the JS</param>
			/// <returns>The name of JS</returns>
			String^ GetJSName(int index)
			{
				return Document_getJSName(m_doc, index);
			}
			/// <summary>
			/// Get if the file can be saved
			/// </summary>
			property Boolean CanSave
			{
				Boolean get() {return Document_canSave( m_doc );}
			}
			/// <summary>
			/// Get if the file is encrypted
			/// </summary>
			property Boolean IsEncrypted
			{
				Boolean get() {return Document_isEncrypted( m_doc );}
			}
			/// <summary>
			/// Get if the file is opened
			/// </summary>
			property Boolean IsOpened
			{
				Boolean get() { return m_doc != NULL; }
			}
			/// <summary>
			/// Get PDFObj with reference
			/// </summary>
			/// <param name="ref">Reference to target PDFObj</param>
			/// <returns>A PDFObj object if found</returns>
			PDFObj ^Advance_GetObj(PDFRef ref)
			{
				PDF_OBJ obj = Document_advGetObj(m_doc, ref.ref);
				if (!obj) return nullptr;
				PDFObj ^ret = ref new PDFObj();
				ret->m_obj = obj;
				return ret;
			}
			/// <summary>
			/// Get reference of current PDFObj
			/// </summary>
			/// <returns>Reference of current PDFObj</returns>
			PDFRef Advance_GetRef()
			{
				PDF_OBJ_REF ref = Document_advGetRef(m_doc);
				PDFRef ret;
				ret.ref = ref;
				return ret;
			}
			/// <summary>
			/// Advanced function to create an empty indirect object to edit. This method require premium license.
			/// </summary>
			/// <returns>Reference to created object</returns>
			PDFRef Advance_NewIndirect()
			{
				PDF_OBJ_REF ref = Document_advNewIndirectObj(m_doc);
				PDFRef ret;
				ret.ref = ref;
				return ret;
			}
			/// <summary>
			/// Advanced function to create an indirect object, and then copy source object to this indirect object. This method require premium license.
			/// </summary>
			/// <param name="obj">Source object to be copied</param>
			/// <returns>Reference to created object</returns>
			PDFRef Advance_NewIndirectAndCopy(PDFObj ^obj)
			{
				PDF_OBJ_REF ref = Document_advNewIndirectObjWithData(m_doc, obj->m_obj);
				PDFRef ret;
				ret.ref = ref;
				return ret;
			}
			/// <summary>
			/// Advanced function to reload document objects. This method require premium license.
			/// </summary>
			void Advance_Reload()
			{
				Document_advReload(m_doc);
			}
			/// <summary>
			/// Advanced function to create a stream using zflate compression(zlib).
			/// Stream byte contents can't modified, once created.
			/// The byte contents shall auto compress and encrypt by native library.
			/// This method require premium license, and need Document.SetCache() invoked.
			/// </summary>
			/// <param name="src">The source data to create the stream</param>
			/// <returns>Reference to created object</returns>
			PDFRef Advance_NewFlateStream(const Array<BYTE> ^src)
			{
				int len = src->Length;
				const unsigned char *data = src->Data;
				PDF_OBJ_REF ref = Document_advNewFlateStream(m_doc, data, len);
				PDFRef ret;
				ret.ref = ref;
				return ret;
			}
			/// <summary>
			/// Advanced function to create a stream using raw data.
			/// If you pass compressed data to this method, you shall modify dictionary of this stream.
			/// Like "Filter" and other item from dictionary.
			/// The byte contents shall auto encrypt by native library, if document if encrypted.
			/// This method require premium license, and need Document.SetCache() invoked.
			/// </summary>
			/// <param name="src">The source data to create the stream</param>
			/// <returns>Reference to created object</returns>
			PDFRef Advance_NewRawStream(const Array<BYTE> ^src)
			{
				int len = src->Length;
				const unsigned char *data = src->Data;
				PDF_OBJ_REF ref = Document_advNewRawStream(m_doc, data, len);
				PDFRef ret;
				ret.ref = ref;
				return ret;
			}
			int VerifySign(PDFSign ^sign);
			
		private:
			class PDFStreamInner:public IPDFStream
			{
			public:
				void Open( PDFStream ^stream )
				{
					m_stream = stream;
				}
				virtual bool Writeable() const
				{
					return m_stream->Writeable();
				}
				virtual unsigned long long GetLen() const
				{
					return m_stream->GetLength();
				}
				virtual unsigned long long GetPos() const
				{
					return m_stream->GetPosition();
				}
				virtual bool SetPos( unsigned long long off )
				{
					return m_stream->SetPosition(off);
				}
				virtual unsigned int Read( void *pBuf, unsigned int dwBuf )
				{
					ArrayReference<BYTE> tmp((BYTE*)pBuf, dwBuf);
					return m_stream->Read( tmp );
				}
				virtual unsigned int Write( const void *pBuf, unsigned int dwBuf )
				{
					ArrayReference<BYTE> tmp((BYTE*)pBuf, dwBuf);
					return m_stream->Write( tmp );
				}
				virtual void Close()
				{
					m_stream->Close();
					m_stream = nullptr;
				}
				virtual void Flush()
				{
					m_stream->Flush();
				}
			protected:
				PDFStream ^m_stream;
			};
			class PDFJSDelegateInner : public IPDFJSDelegate
			{
			public:
				PDFJSDelegateInner(PDFJSDelegate ^del)
				{
					m_del = del;
				}
				/// <summary>
				/// Console command
				/// </summary>
				/// <param name="cmd">
				/// *0: clear console.
				/// *1: hide console.
				///	*2: print line on sonsole.
				///	*3: show console.</param>
				/// <param name="para">Only valid when cmd == 2;</param>
				virtual void OnConsole(int cmd, const char *para)
				{
					int max = strlen(para) + 1;
					wchar_t *wstmp = (wchar_t *)malloc(sizeof(wchar_t) * max);
					::MultiByteToWideChar(CP_ACP, 0, para, -1, wstmp, max);
					String ^tmp = ref new String(wstmp);
					free(wstmp);
					m_del->OnConsole(cmd, tmp);
				}
				/// <summary>
				/// Show a alert dialog on screen.
				/// </summary>
				/// <param name="btn">uttons show on dialog.
				/// *0: OK
				///	*1: OK, Cancel
				///	*2: Yes, No
				///	*3: Yes, No, Cancel</param>
				/// <param name="msg">Message to be show.</param>
				/// <param name="title">Title to be show.</param>
				/// <returns>Tbutton user clicked. Values as below:
				/// *1: OK
				///	*2: Cancel
				///	*3: No
				///	*4: Yes</returns>
				virtual int OnAlert(int btn, const char *msg, const char *title)
				{
					int max_msg = strlen(msg) + 1;
					wchar_t *wsmsg = (wchar_t *)malloc(sizeof(wchar_t) * max_msg);
					::MultiByteToWideChar(CP_ACP, 0, msg, -1, wsmsg, max_msg);
					String ^tmp_msg = ref new String(wsmsg);
					free(wsmsg);

					int max_title = strlen(title) + 1;
					wchar_t *wstitle = (wchar_t *)malloc(sizeof(wchar_t) * max_title);
					::MultiByteToWideChar(CP_ACP, 0, title, -1, wstitle, max_title);
					String ^tmp_title = ref new String(wstitle);
					free(wstitle);
					return m_del->OnAlert(btn, tmp_msg, tmp_title);
				}
				/// <summary>
				/// Callback when document closed.
				/// </summary>
				/// <returns>True is the Document object need save, otherwise false.</returns>
				virtual bool OnDocClose()
				{
					return m_del->OnDocClose();
				}
				/// <summary>
				/// Generate a tmp file name that JS needed in background.
				/// </summary>
				/// <returns>Absolute path to temp path generated.</returns>
				virtual char *OnTmpFile()
				{
					String ^tmp = m_del->OnTmpFile();
					const wchar_t *wstmp = tmp->Data();
					int len = wcslen(wstmp) + 1;
					char *stmp = (char *)malloc(len * sizeof(wchar_t));
					::WideCharToMultiByte(CP_ACP, 0, wstmp, -1, stmp, len * sizeof(wchar_t), NULL, NULL);
					return stmp;
				}
				/// <summary>
				/// Callback when an Uncaught exception appears.
				/// </summary>
				/// <param name="code">Error code.</param>
				/// <param name="msg">Error message.</param>
				virtual void OnUncaughtException(int code, const char *msg)
				{
					int max = strlen(msg) + 1;
					wchar_t *wstmp = (wchar_t *)malloc(sizeof(wchar_t) * max);
					::MultiByteToWideChar(CP_ACP, 0, msg, -1, wstmp, max);
					String ^tmp = ref new String(wstmp);
					free(wstmp);
					m_del->OnUncaughtException(code, tmp);
				}
			private:
				PDFJSDelegate ^m_del;
			};
			friend PDFDocFont;
			friend PDFDocGState;
			friend PDFDocForm;
			friend PDFImportCtx;
			friend PDFOutline;
			~PDFDoc();
			PDF_DOC m_doc;
			PDFStreamInner *m_inner;
		};
		public ref class PDFSign sealed
		{
		public:
			/// <summary>
			/// Get the issuer of the signature
			/// </summary>
			property String ^Issue
			{
				String ^get()
				{
					return Sign_getIssue(m_sign);
				}
			}
			/// <summary>
			/// Get the subject of the signature
			/// </summary>
			property String ^Subject
			{
				String ^get()
				{
					return Sign_getSubject(m_sign);
				}
			}
			/// <summary>
			/// Get the version of the signature
			/// </summary>
			property long long Version
			{
				long long get()
				{
					return Sign_getVersion(m_sign);
				}
			}
			/// <summary>
			/// Get the name of the signature
			/// </summary>
			property String^ Name
			{
				String^ get()
				{
					return Sign_getName(m_sign);
				}
			}
			/// <summary>
			/// Get the reason for issuing the signature
			/// </summary>
			property String^ Reason
			{
				String^ get()
				{
					return Sign_getReason(m_sign);
				}
			}
			/// <summary>
			/// Get the location for issuing the signature
			/// </summary>
			property String^ Location
			{
				String^ get()
				{
					return Sign_getLocation(m_sign);
				}
			}
			/// <summary>
			/// Get the contact of issuer
			/// </summary>
			property String^ Contact
			{
				String^ get()
				{
					return Sign_getContact(m_sign);
				}
			}
			/// <summary>
			/// Get the last modify time.
			/// </summary>
			property String^ ModTime
			{
				String^ get()
				{
					return Sign_getModDT(m_sign);
				}
			}
		private:
			PDFSign()
			{
				m_sign = NULL;
			}
			friend PDFDoc;
			friend PDFPage;
			friend PDFDocForm;
			friend PDFAnnot;
			PDF_SIGN m_sign;
		};
		public ref class PDFDocImage sealed
		{
		public:
		private:
			PDFDocImage()
			{
				m_image = NULL;
			}
			friend PDFDoc;
			friend PDFPage;
			friend PDFDocForm;
			PDF_DOC_IMAGE m_image;
		};
		public ref class PDFDocFont sealed
		{
		public:
			/// <summary>
			/// Get the ascent of the font
			/// </summary>
			property float Ascent
			{
				float get(){return Document_getFontAscent(m_doc->m_doc, m_font);}
			}
			/// <summary>
			/// Get the descent of the font
			/// </summary>
			property float Descent
			{
				float get(){return Document_getFontDescent(m_doc->m_doc, m_font);}
			}
		private:
			PDFDocFont()
			{
				m_font = NULL;
			}
			friend PDFDoc;
			friend PDFPage;
			friend PDFDocForm;
			friend PDFAnnot;
			friend PDFPageContent;
			PDFDoc ^m_doc;
			PDF_DOC_FONT m_font;
		};
		public ref class PDFDocGState sealed
		{
		public:
			/// <summary>
			/// Set the alpha value for fill in shapes
			/// </summary>
			/// <param name="alpha">The alpha value</param>
			void SetFillAlpha(int alpha)
			{
				Document_setGStateFillAlpha(m_doc->m_doc, m_gs, alpha);
			}
			/// <summary>
			/// Set the alpha value for stroke
			/// </summary>
			/// <param name="alpha">The alpha value</param>
			void SetStrokeAlpha(int alpha)
			{
				Document_setGStateStrokeAlpha(m_doc->m_doc, m_gs, alpha);
			}
			/// <summary>
			/// Set the dash pattern for stroke
			/// </summary>
			/// <param name="dash">The dash pattern</param>
			/// <param name="dash_cnt">The count of dash pattern</param>
			/// <param name="phase">The phase of dash pattern</param>
			void SetStrokeDash(const Array<float> ^dash, float phase)
			{
				if(dash)
					Document_setGStateStrokeDash(m_doc->m_doc, m_gs, dash->Data, dash->Length, phase);
				else
					Document_setGStateStrokeDash(m_doc->m_doc, m_gs, NULL, 0, 0);
			}
			/// <summary>
			/// Set blend mode to graphic state.
			/// </summary>
			/// <param name="bmode">bmode 2:Multipy
			/// 3:Screen
			/// 4 : Overlay
			/// 5 : Darken
			/// 6 : Lighten
			/// 7 : ColorDodge
			/// 8 : ColorBurn
			/// 9 : Difference
			/// 10 : Exclusion
			/// 11 : Hue
			/// 12 : Saturation
			/// 13 : Color
			/// 14 : Luminosity
			/// others : Normal</param>
			void SetBlendMode(int bmode)
			{
				Document_setGStateBlendMode(m_doc->m_doc, m_gs, bmode);
			}
		private:
			PDFDocGState()
			{
				m_gs = NULL;
			}
			friend PDFDoc;
			friend PDFPage;
			friend PDFDocForm;
			PDF_DOC_GSTATE m_gs;
			PDFDoc ^m_doc;
		};
		public ref class PDFDocForm sealed
		{
		public:
			PDFPageForm ^AddResForm(PDFDocForm ^sub);
			PDFPageFont ^AddResFont(PDFDocFont ^font);
			PDFPageGState ^AddResGState(PDFDocGState ^gs);
			PDFPageImage ^AddResImage(PDFDocImage ^img);
			void SetContent(PDFPageContent ^content, float x, float y, float w, float h);
			/// <summary>
			/// set this form as transparency.
			/// </summary>
			/// <param name="isolate">set to isolate, mostly are false.</param>
			/// <param name="knockout">set to knockout, mostly are false.</param>
			void SetTransparency(bool isolate, bool knockout)
			{
				Document_setFormTransparency(m_doc->m_doc, m_form, isolate, knockout);
			}
		private:
			PDFDocForm()
			{
				m_form = NULL;
			}
			~PDFDocForm()
			{
				Document_freeForm(m_doc->m_doc, m_form);
			}
			friend PDFDoc;
			friend PDFPage;
			friend PDFAnnot;
			PDFDoc ^m_doc;
			PDF_DOC_FORM m_form;
		};
		public ref class PDFImportCtx sealed
		{
		public:
			/// <summary>
			/// import a page to the document. A premium license is required for this method.
			/// Do not forget to invoke ImportContext.Destroy() after all pages are imported.
			/// </summary>
			/// <param name="srcno">0 based page NO. from source Document that passed to ImportStart.</param>
			/// <param name="dstno">0 based page NO. to insert in this document object.</param>
			/// <returns></returns>
			Boolean ImportPage(int srcno, int dstno)
			{
				return Document_importPage(m_doc->m_doc, m_ctx, srcno, dstno);
			}
		private:
			PDFImportCtx()
			{
				m_ctx = NULL;
				m_doc = nullptr;
			}
			~PDFImportCtx()
			{
				Document_importEnd( m_doc->m_doc, m_ctx );
			}
			friend PDFDoc;
			friend PDFPage;
			PDFDoc ^m_doc;
			PDF_IMPORTCTX m_ctx;
		};
		public ref class PDFOutline sealed
		{
		public:
			PDFOutline ^GetNext();
			PDFOutline ^GetChild();
			Boolean AddNext( String ^label, int dest, float y );
			Boolean AddChild( String ^label, int dest, float y );
			Boolean RemoveFromDoc();
			/// <summary>
			/// Get PDFOutline label
			/// </summary>
			property String^label
			{
				String ^get()
				{
					return Document_getOutlineLabel( m_doc->m_doc, m_outline);
				}
			}
			/// <summary>
			/// Get the target page of the outline item
			/// </summary>
			property int dest
			{
				int get()
				{
					return Document_getOutlineDest( m_doc->m_doc, m_outline );
				}
			}
			property Array<int>^ dest_para
			{
				Array<int>^ get()
				{
					Array<int>^ tmp = ref new Array<int>(7);
					Document_getOutlineDest2(m_doc->m_doc, m_outline, tmp->Data);
					return tmp;
				}
			}
		private:
			PDFOutline()
			{
				m_doc = nullptr;
				m_outline = NULL;
			}
			friend PDFDoc;
			PDFDoc ^m_doc;
			PDF_OUTLINE m_outline;
		};
		public ref class PDFPageGState sealed
		{
		public:
		private:
			PDFPageGState()
			{
				m_gs = NULL;
			}
			friend PDFDocForm;
			friend PDFPage;
			friend PDFPageContent;
			PDF_PAGE_GSTATE m_gs;
		};
		public ref class PDFPageImage sealed
		{
		public:
		private:
			PDFPageImage()
			{
				m_image = NULL;
			}
			friend PDFDocForm;
			friend PDFPage;
			friend PDFPageContent;
			PDF_PAGE_IMAGE m_image;
		};
		public ref class PDFPageFont sealed
		{
		public:
		private:
			PDFPageFont()
			{
				m_font = NULL;
			}
			friend PDFDocForm;
			friend PDFPage;
			friend PDFPageContent;
			PDF_PAGE_FONT m_font;
		};
		public ref class PDFPageForm sealed
		{
		public:
		private:
			PDFPageForm()
			{
				m_form = NULL;
			}
			friend PDFDocForm;
			friend PDFPage;
			friend PDFPageContent;
			PDF_PAGE_FORM m_form;
		};
		public ref class PDFPageContent sealed
		{
		public:
			PDFPageContent()
			{
				m_content = PageContent_create();
			}
			/// <summary>
			/// Save current graphic status
			/// </summary>
			void GSSave()
			{
				PageContent_gsSave( m_content );
			}
			/// <summary>
			/// Restore graphic to previous status
			/// </summary>
			void GSRestore()
			{
				PageContent_gsRestore( m_content );
			}
			/// <summary>
			/// Set a new graphic status
			/// </summary>
			/// <param name="gs">New graphic status to set</param>
			void GSSet( PDFPageGState ^gs )
			{
				PageContent_gsSet( m_content, gs->m_gs );
			}
			/// <summary>
			/// Set a PDFMatrix, which can be used for coordinate transforming
			/// </summary>
			/// <param name="mat"></param>
			void GSSetMatrix( PDFMatrix ^mat )
			{
				PageContent_gsSetMatrix( m_content, mat->m_mat );
			}
			/// <summary>
			/// Start a text output process
			/// </summary>
			void TextBegin()
			{
				PageContent_textBegin( m_content );
			}
			/// <summary>
			/// End a text output process
			/// </summary>
			void TextEnd()
			{
				PageContent_textEnd( m_content );
			}
			/// <summary>
			/// Draw an image
			/// </summary>
			/// <param name="img">Image data to draw</param>
			void DrawImage( PDFPageImage ^img )
			{
				PageContent_drawImage( m_content, img->m_image );
			}
			/// <summary>
			/// Draw a PDFPageForm object
			/// </summary>
			/// <param name="form">PDFPageForm object to draw</param>
			void DrawImage(PDFPageForm ^form)
			{
				PageContent_drawForm(m_content, form->m_form);
			}
			/// <summary>
			/// Output text
			/// </summary>
			/// <param name="text">Text to draw on the page</param>
			void DrawText(String ^text)
			{
				PageContent_drawTextW( m_content, text->Data() );
			}
			/// <summary>
			/// Output text
			/// </summary>
			/// <param name="text">Text to draw on the page</param>
			/// <param name="align">Alignment of the text</param>
			/// <param name="width">Max width to draw the text</param>
			/// <returns>Lines of the output text</returns>
			int DrawText(String^ text, int align, float width)
			{
				return PageContent_drawText2W(m_content, text->Data(), align, width);
			}
			/// <summary>
			/// Output text
			/// </summary>
			/// <param name="text">Text to draw on the page</param>
			/// <param name="align">Alignment of the text</param>
			/// <param name="width">Max width to draw the text</param>
			/// <param name="max_lines">Max lines to draw the text</param>
			/// <returns>A PDFTextRet object to describe the rectangle which was used to draw the text</returns>
			PDFTextRet DrawText(String^ text, int align, float width, int max_lines)
			{
				int val = PageContent_drawText3W(m_content, text->Data(), align, width, max_lines);
				PDFTextRet ret;
				ret.num_unicodes = val & ((1 << 20) - 1);
				ret.num_lines = val >> 20;
				return ret;
			}
			/// <summary>
			/// Measure total width of output string
			/// </summary>
			/// <param name="text">Text to measure</param>
			/// <param name="pfont">Font will be used to draw the text</param>
			/// <param name="width">Width of individual character</param>
			/// <param name="height">Height of individual character</param>
			/// <param name="char_space">Width of space between characters.</param>
			/// <param name="word_space">Width of space between words.</param>
			/// <returns>A PDFPoint object describes the width and height of the string</returns>
			PDFPoint GetTextSize(String ^text, PDFPageFont ^pfont, float width, float height, float char_space, float word_space)
			{
				PDF_POINT pt = PageContent_textGetSizeW(m_content, pfont->m_font, text->Data(), width, height, char_space, word_space);
				return *(PDFPoint *)&pt;
			}
			/// <summary>
			/// Draw a path object with stroke
			/// </summary>
			/// <param name="path">Path object to draw</param>
			void StrokePath( PDFPath ^path )
			{
				PageContent_strokePath( m_content, path->m_path );
			}
			/// <summary>
			/// Draw a path object and fill
			/// </summary>
			/// <param name="path">Path object to draw</param>
			void FillPath( PDFPath ^path, Boolean winding )
			{
				PageContent_fillPath( m_content, path->m_path, winding );
			}
			/// <summary>
			/// Set the path as clip path.
			/// </summary>
			/// <param name="path">Path object to draw</param>
			/// <param name="winding">Winding fill rule</param>
			void ClipPath( PDFPath ^path, Boolean winding )
			{
				PageContent_clipPath( m_content, path->m_path, winding );
			}
			/// <summary>
			/// PDF operator: set fill and other operations color.
			/// </summary>
			/// <param name="color">Formatted as 0xRRGGBB, no alpha channel. alpha value shall set by ExtGraphicState(ResGState).</param>
			void SetFillColor( int color )
			{
				PageContent_setFillColor(m_content, color);
			}
			/// <summary>
			/// PDF operator: set stroke color.
			/// </summary>
			/// <param name="color">Formatted as 0xRRGGBB, no alpha channel. alpha value shall set by ExtGraphicState(ResGState).</param>
			void SetStrokeColor( int color )
			{
				PageContent_setStrokeColor(m_content, color);
			}
			/// <summary>
			/// PDF operator: set line cap
			/// </summary>
			/// <param name="cap">0:butt, 1:round: 2:square</param>
			void SetStrokeCap( int cap )
			{
				PageContent_setStrokeCap(m_content, cap);
			}
			/// <summary>
			/// PDF operator: set line join
			/// </summary>
			/// <param name="join">0:miter, 1:round, 2:bevel</param>
			void SetStrokeJoin( int join )
			{
				PageContent_setStrokeJoin(m_content, join);
			}
			/// <summary>
			/// PDF operator: set line width
			/// </summary>
			/// <param name="w">Line width in PDF coordinate</param>
			void SetStrokeWidth( float w )
			{
				PageContent_setStrokeWidth(m_content, w);
			}
			/// <summary>
			/// PDF operator: set miter limit.
			/// </summary>
			/// <param name="miter">Miter limit.</param>
			void SetStrokeMiter( float miter )
			{
				PageContent_setStrokeMiter(m_content, miter);
			}
			/// <summary>
			/// Set dash for stroke operation. Please refer to PDF-Reference 1.7 (4.3.2) Line Dash Pattern for more details.
			/// </summary>
			/// <param name="dash">Dash array, if null, means set to solid.</param>
			/// <param name="phase">Phase value, mostly, it is 0.</param>
			void SetStrokeDash(const Array<float>^ dash, float phase)
			{
				if (dash && dash->Length > 0)
					PageContent_setStrokeDash(m_content, dash->Data, dash->Length, phase);
				else
					PageContent_setStrokeDash(m_content, NULL, 0, phase);
			}
			/// <summary>
			/// PDF operator: set char space(extra space between chars).
			/// </summary>
			/// <param name="space">Char space</param>
			void TextSetCharSpace( float space )
			{
				PageContent_textSetCharSpace( m_content, space );
			}
			/// <summary>
			/// PDF operator: set word space(extra space between words spit by blank char ' ' ).
			/// </summary>
			/// <param name="space">Word space.</param>
			void TextSetWordSpace( float space )
			{
				PageContent_textSetWordSpace( m_content, space );
			}
			/// <summary>
			/// PDF operator: set text leading, height between 2 text lines.
			/// </summary>
			/// <param name="leading">Leading in PDF coordinate</param>
			void TextSetLeading( float leading )
			{
				PageContent_textSetLeading( m_content, leading );
			}
			/// <summary>
			/// PDF operator: set text rise
			/// </summary>
			/// <param name="rise">Text rise</param>
			void TextSetRise( float rise )
			{
				PageContent_textSetRise( m_content, rise );
			}
			/// <summary>
			/// PDF operator: set horizon scale for chars.
			/// </summary>
			/// <param name="scale">Char scale, in percentage. Set to 100 means scale value 1.0f</param>
			void TextSetHScale( int scale )
			{
				PageContent_textSetHScale( m_content, scale );
			}
			/// <summary>
			/// PDF operator: new a text line
			/// </summary>
			void TextNextLine()
			{
				PageContent_textNextLine( m_content );
			}
			/// <summary>
			/// PDF operator: move text position relative to previous line
			/// </summary>
			/// <param name="x">x in PDF coordinate add to previous line position</param>
			/// <param name="y">y in PDF coordinate add to previous line position</param>
			void TextMove( float x, float y )
			{
				PageContent_textMove( m_content, x, y );
			}
			/// <summary>
			/// Set text font
			/// </summary>
			/// <param name="font">ResFont object created by Page.AddResFont() or Form.AddResFont()</param>
			/// <param name="size">Text size in PDF coordinate.</param>
			void TextSetFont( PDFPageFont ^font, float size )
			{
				PageContent_textSetFont( m_content, font->m_font, size );
			}
			/// <summary>
			/// PDF operator: set text render mode.
			/// </summary>
			/// <param name="mode">0: filling
			/// 1: stroke
			/// 2 : fill and stroke
			/// 3 : do nothing
			/// 4 : fill and set clip path
			/// 5 : stroke and set clip path
			/// 6 : fill / stroke / clip
			/// 7 : set clip path.</param>
			void TextSetRenderMode( int mode )
			{
				PageContent_textSetRenderMode( m_content, mode );
			}
		private:
			friend PDFPage;
			friend PDFDocForm;
			~PDFPageContent()
			{
				PageContent_destroy( m_content );
			}
			PDF_PAGECONTENT m_content;
		};
		public ref class PDFFinder sealed
		{
		public:
			/// <summary>
			/// Get find count in current page.
			/// </summary>
			/// <returns>Count or 0 if no found.</returns>
			int GetCount()
			{
				return Page_findGetCount( m_finder );
			}
			/// <summary>
			/// Get first char index.
			/// </summary>
			/// <param name="index">0 based index value. range:[0, FindGetCount()-1]</param>
			/// <returns>The first char index of texts, see: ObjsGetString. range:[0, ObjsGetCharCount()-1]</returns>
			int GetFirstChar( int index )
			{
				return Page_findGetFirstChar( m_finder, index );
			}
			/// <summary>
			/// Get last char index.
			/// </summary>
			/// <param name="index">0 based index value. range:[0, FindGetCount()-1]</param>
			/// <returns>The last char index of texts, see: ObjsGetString. range:[0, ObjsGetCharCount()-1]</returns>
			int GetLastChar(int index)
			{
				return Page_findGetEndChar(m_finder, index);
			}
		private:
			PDFFinder()
			{
				m_finder = 0;
			}
			~PDFFinder()
			{
				Page_findClose( m_finder );
			}
			friend PDFPage;
			PDF_FINDER m_finder;
		};
		public ref class PDFPage sealed
		{
		public:
			/// <summary>
			/// Get rotated CropBox.
			/// </summary>
			property PDFRect CropBox
			{
				PDFRect get()
				{
					PDF_RECT rc;
					Page_getCropBox( m_page, &rc );
					return *(PDFRect *)&rc;
				}
			}
			/// <summary>
			/// Get rotated MediaBox.
			/// </summary>
			property PDFRect MediaBox
			{
				PDFRect get()
				{
					PDF_RECT rc;
					Page_getMediaBox( m_page, &rc );
					return *(PDFRect *)&rc;
				}
			}
			/// <summary>
			/// Import annotation from memory(byte array).
			/// A premium license is required for this method.
			/// </summary>
			/// <param name="rect">[left, top, right, bottom] in PDF coordinate. which is the import annotation's position.</param>
			/// <param name="buf">data returned from Annotation.Export()</param>
			/// <param name="buf_len">Length of data</param>
			/// <returns>True if successed, otherwise false</returns>
			bool ImportAnnot(PDFRect rect, const Array<BYTE> ^buf, int buf_len)
			{
				return Page_importAnnot(m_page, (PDF_RECT *)&rect, buf->Data, buf_len);
			}
			/// <summary>
			/// Start Reflow.
			/// This method require professional or premium license
			/// </summary>
			/// <param name="width">Input width, height property will be calculated and returned by this method.</param>
			/// <param name="ratio">Scale base to 72 DPI, 2.0 means 144 DPI. the reflowed text will displayed in scale</param>
			/// <param name="reflow_images">Enable/Disable reflow images.</param>
			/// <returns>The height that reflow needed.</returns>
			float ReflowStart(float width, float ratio, bool reflow_images)
			{
				return Page_reflowStart(m_page, width, ratio, reflow_images);
			}
			/// <summary>
			/// Reflow to dib.
			/// This method require professional or premium license
			/// </summary>
			/// <param name="dib">PDFDIB to render</param>
			/// <param name="orgx">Origin x coordinate</param>
			/// <param name="orgy">Origin y coordinate</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean Reflow(PDFDIB ^dib, float orgx, float orgy)
			{
				return Page_reflow(m_page, dib->m_dib, orgx, orgy);
			}
			/// <summary>
			/// Reflow to Bitmap object.
			/// This method require professional or premium license
			/// </summary>
			/// <param name="bmp">Bitmap to reflow</param>
			/// <param name="orgx">origin x coordinate</param>
			/// <param name="orgy">origin y coordinate</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean ReflowToBmp(PDFBmp ^bmp, float orgx, float orgy)
			{
				return Page_reflowToBmp(m_page, bmp->m_bmp, orgx, orgy);
			}
			/// <summary>
			/// Prepare to render. it reset dib pixels to white value, and reset page status.
			/// If dib is null, only to reset page status.
			/// </summary>
			/// <param name="dib">DIB object to render. get from Global.dibGet() or null.</param>
			void RenderPrepare( PDFDIB ^dib )
			{
				Page_renderPrepare( m_page, dib->m_dib );
			}
			/// <summary>
			/// Prepare to render.
			/// </summary>
			void RenderPrepare()
			{
				Page_renderPrepare( m_page, NULL );
			}
			/// <summary>
			/// Render page to PDFDIB object. this function returned for cancelled or finished.
			/// Before render, RenderPrepare() method must be invoked.
			/// </summary>
			/// <param name="dib">PDFDIB object to render to</param>
			/// <param name="mat">Matrix object define scale, rotate, translate operations.</param>
			/// <param name="show_annot">Show/Hide annotations</param>
			/// <param name="mode">Specifies the render quality. Please refer to definition of PDF_RENDER_MODE</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean Render( PDFDIB ^dib, PDFMatrix ^mat, Boolean show_annot, PDF_RENDER_MODE mode )
			{
				return Page_render( m_page, dib->m_dib, mat->m_mat, show_annot, (::PDF_RENDER_MODE)mode );
			}
			/// <summary>
			/// Render page to Bitmap object directly. this function returned for cancelled or finished.
			/// Bitmap object must be erased before rendering.
			/// </summary>
			/// <param name="bmp">Bitmap object to render to.</param>
			/// <param name="mat">Matrix object define scale, rotate, translate operations.</param>
			/// <param name="show_annot">Show/Hide annotations</param>
			/// <param name="mode">Specifies the render quality. Please refer to definition of PDF_RENDER_MODE</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean RenderToBmp( PDFBmp ^bmp, PDFMatrix ^mat, Boolean show_annot, PDF_RENDER_MODE mode )
			{
				return Page_renderToBmp( m_page, bmp->m_bmp, mat->m_mat, show_annot, (::PDF_RENDER_MODE)mode );
			}
			/// <summary>
			/// Render page to PDFSoftBmp object directly. this function returned for cancelled or finished.
			/// </summary>
			/// <param name="bmp">PDFSoftBmp object to render to.</param>
			/// <param name="mat">Matrix object define scale, rotate, translate operations.</param>
			/// <param name="show_annot">Show/Hide annotations</param>
			/// <param name="mode">Specifies the render quality. Please refer to definition of PDF_RENDER_MODE</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean RenderToSoftBmp( PDFSoftBmp^ bmp, PDFMatrix^ mat, Boolean show_annot, PDF_RENDER_MODE mode)
			{
				return Page_renderToBmp(m_page, bmp->m_bmp, mat->m_mat, show_annot, (::PDF_RENDER_MODE)mode);
			}
			/// <summary>
			/// Set page status to cancelled and cancel render function.
			/// </summary>
			void RenderCancel()
			{
				Page_renderCancel( m_page );
			}
			/// <summary>
			/// Get if page rendering is finished.
			/// </summary>
			/// <returns>True if rendering is finished, otherwise false</returns>
			Boolean RenderIsFinished()
			{
				return Page_renderIsFinished( m_page );
			}
			/// <summary>
			/// Add a font as resource of this page.
			/// A premium license is required for this method.
			/// </summary>
			/// <param name="font">Font object created by Document.NewFontCID()</param>
			/// <returns>Created PDFPageFont object, or null if failed to add resource</returns>
			PDFPageFont ^AddResFont( PDFDocFont ^font )
			{
				PDF_PAGE_FONT pf = Page_addResFont( m_page, font->m_font );
				if( pf )
				{
					PDFPageFont ^font = ref new PDFPageFont();
					font->m_font = pf;
					return font;
				}
				else return nullptr;
			}
			/// <summary>
			/// Add an image as resource of this page.
			/// A premium license is required for this method.
			/// </summary>
			/// <param name="image">Image object created by Document.NewImage() or Document.NewImageJPEG()</param>
			/// <returns>Generated PDFPageImage object, or null if failed</returns>
			PDFPageImage ^AddResImage( PDFDocImage ^image )
			{
				PDF_PAGE_IMAGE pf = Page_addResImage( m_page, image->m_image );
				if( pf )
				{
					PDFPageImage ^font = ref new PDFPageImage();
					font->m_image = pf;
					return font;
				}
				else return nullptr;
			}
			/// <summary>
			/// Add GraphicState as resource of this page.
			/// A premium license is required for this method.
			/// </summary>
			/// <param name="gs"></param>
			/// <returns>Generated PDFPageGState object, or null if failed</returns>
			PDFPageGState ^AddResGState( PDFDocGState ^gs )
			{
				PDF_PAGE_GSTATE pf = Page_addResGState( m_page, gs->m_gs );
				if( pf )
				{
					PDFPageGState ^font = ref new PDFPageGState();
					font->m_gs = pf;
					return font;
				}
				else return nullptr;
			}
			/// <summary>
			/// Add Form as resource of this page.
			/// A premium license is required for this method.
			/// </summary>
			/// <param name="form">Form created by Document.NewForm();</param>
			/// <returns>Generated PDFPageForm object, or null if failed</returns>
			PDFPageForm ^AddResForm(PDFDocForm ^form)
			{
				PDF_PAGE_FORM pf = Page_addResForm(m_page, form->m_form);
				if (pf)
				{
					PDFPageForm ^font = ref new PDFPageForm();
					font->m_form = pf;
					return font;
				}
				else return nullptr;
			}
			/// <summary>
			/// Add content stream to this page.
			/// A premium license is required for this method.
			/// </summary>
			/// <param name="content">PageContent object called PageContent.create().</param>
			/// <param name="flush">does need flush all resources?
			/// True, if you want render page after this method, or false.
			/// If false, added texts won't displayed till Document.Save() or Document.SaveAs() invoked.</param>
			/// <returns>True if success, otherwise false</returns>
			Boolean AddContent(PDFPageContent ^content, Boolean flush)
			{
				return Page_addContent( m_page, content->m_content, flush );
			}
			/// <summary>
			/// Get text objects to memory.
			/// A standard license is required for this method
			/// </summary>
			void ObjsStart()
			{
				Page_objsStart( m_page );
			}
			/// <summary>
			/// Get chars count in this page. this can be invoked after ObjsStart
			/// A standard license is required for this method
			/// </summary>
			/// <returns>Char count or 0 if ObjsStart not invoked.</returns>
			int ObjsGetCharCount()
			{
				return Page_objsGetCharCount(m_page);
			}
			/// <summary>
			/// Get char's box in PDF coordinate system, this can be invoked after ObjsStart
			/// </summary>
			/// <param name="index">0 based unicode index.</param>
			/// <returns>A PDFRect object for PDF rectangle.</returns>
			PDFRect ObjsGetCharRect( int index )
			{
				PDFRect rect;
				Page_objsGetCharRect( m_page, index, (PDF_RECT *)&rect );
				return rect;
			}
			/// <summary>
			/// Get char index nearest to point
			/// </summary>
			/// <param name="x">x in PDF coordinate</param>
			/// <param name="y">y in PDF coordinate</param>
			/// <returns>Char index or -1 if failed</returns>
			int ObjsGetCharIndex( float x, float y )
			{
				return Page_objsGetCharIndex( m_page, x, y );
			}
			/// <summary>
			/// Get index aligned by word. this can be invoked after ObjsStart
			/// </summary>
			/// <param name="index">0 based unicode index.</param>
			/// <param name="dir">If dir < 0,  get start index of the word. otherwise get last index of the word.</param>
			/// <returns>New index value.</returns>
			int ObjsAlignWord( int index, int dir )
			{
				return Page_objsAlignWord( m_page, index, dir );
			}
			/// <summary>
			/// Get char's font name. this can be invoked after ObjsStart
			/// </summary>
			/// <param name="index">0 based unicode index.</param>
			/// <returns>Font name, may be null.</returns>
			String ^ObjsGetCharFontName( int index )
			{
				return cvt_cstr_str( Page_objsGetCharFontName( m_page, index ) );
			}
			/// <summary>
			/// Get string from range. this can be invoked after ObjsStart
			/// </summary>
			/// <param name="from">0 based unicode index.</param>
			/// <param name="to">0 based unicode index.</param>
			/// <returns>String or null</returns>
			String ^ObjsGetString( int from, int to )
			{
				wchar_t *txt = (wchar_t *)malloc( sizeof( wchar_t ) * (to - from + 3) );
				Page_objsGetStringW( m_page, from, to, txt, to - from + 2 );
				String ^ret = ref new String( txt );
				free( txt );
				return ret;
			}
			/// <summary>
			/// Create a PDFFinder
			/// </summary>
			/// <param name="key">Key to search</param>
			/// <param name="match_case">Search with case sensitive or not.</param>
			/// <param name="whole_word">Search with whole word match or not.</param>
			/// <returns>PDFFinder generated, or nullptr if failed</returns>
			PDFFinder ^GetFinder( String ^key, Boolean match_case, Boolean whole_word )
			{
				PDF_FINDER find = Page_findOpenW( m_page, key->Data(), match_case, whole_word );
				if( find )
				{
					PDFFinder ^finder = ref new PDFFinder();
					finder->m_finder = find;
					return finder;
				}
				else return nullptr;
			}
			/// <summary>
			/// Create a PDFFinder. Must be invoke after ObjsStart().
			/// Line break is treat as blank character.
			/// </summary>
			/// <param name="key">Key to search</param>
			/// <param name="match_case">Search with case sensitive or not.</param>
			/// <param name="whole_word">Search with whole word match or not.</param>
			/// <param name="skip_blanks">Search skip blank character or not.</param>
			/// <returns>PDFFinder generated, or nullptr if failed</returns>
			PDFFinder^ GetFinder(String^ key, Boolean match_case, Boolean whole_word, Boolean skip_blanks)
			{
				PDF_FINDER find = Page_findOpen2W(m_page, key->Data(), match_case, whole_word, skip_blanks);
				if (find)
				{
					PDFFinder^ finder = ref new PDFFinder();
					finder->m_finder = find;
					return finder;
				}
				else return nullptr;
			}
			PDFAnnot ^GetAnnot( int index );
			PDFAnnot ^GetAnnot( float x, float y );
			/// <summary>
			/// Remove all annotations and display it as normal content on page.
			/// This method require premium license.
			/// </summary>
			/// <returns>True if successed, otherwise false.</returns>
			Boolean FlatAnnots()
			{
				return Page_flate(m_page);
			}
			/// <summary>
			/// Get annotation count in the page.
			/// </summary>
			property int AnnotCount
			{
				int get(){return Page_getAnnotCount(m_page);}
			}
			/// <summary>
			/// Get rotate degree for page, example: 0 or 90
			/// </summary>
			property int Rotate
			{
				int get() { return Page_getRotate(m_page); }
			}
			/// <summary>
			/// Add an annotation to the page. Page should be re-rendered to refresh changes after updated.
			/// This method can be invoked after ObjsStart or Render or RenderToBmp.
			/// This method require professional or premium license.
			/// </summary>
			/// <param name="ref">Reference to annotation</param>
			/// <param name="index">Index to add the annotation</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean AddAnnot(PDFRef ref, int index)
			{
				return Page_addAnnot(m_page, ref.ref, index);
			}
			/// <summary>
			/// Add a text-markup annotation to page.
			/// You should re - render page to display modified data.
			/// This can be invoked after ObjsStart or Render or RenderToBmp.
			/// This method require professional or premium license
			/// </summary>
			/// <param name="ci1">Index of start character</param>
			/// <param name="ci2">Index of end character</param>
			/// <param name="color">Color of the markup</param>
			/// <param name="type">Markup type:
			/// 0: Highlight
			/// 1: Underline
			/// 2: StrikeOut
			/// 3: Highlight without round corner.
			/// </param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean AddAnnotMarkup( int ci1, int ci2, unsigned int color, int type )
			{
				return Page_addAnnotMarkup2( m_page, ci1, ci2, color, type );
			}
			/// <summary>
			/// Add goto-page link to page.
			/// You should re - render page to display modified data.
			/// This can be invoked after ObjsStart or Render or RenderToBmp.
			/// This method require professional or premium license
			/// </summary>
			/// <param name="rect">Link area rect [left, top, right, bottom] in PDF coordinate.</param>
			/// <param name="dest">0 based page number to goto.</param>
			/// <param name="y">y coordinate in PDF coordinate, page.height is top of page. and 0 is bottom of page.</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean AddAnnotGoto( PDFRect rect, int dest, float y )
			{
				return Page_addAnnotGoto2( m_page, (const PDF_RECT *)&rect, dest, y );
			}
			/// <summary>
			/// Add URL link to page.
			/// You should re - render page to display modified data.
			/// This can be invoked after ObjsStart or Render or RenderToBmp.
			/// This method require professional or premium license
			/// The added annotation can be obtained by Page.GetAnnot(Page.GetAnnotCount() - 1), if this method return true.
			/// </summary>
			/// <param name="rect">Link area rect [left, top, right, bottom] in PDF coordinate.</param>
			/// <param name="uri">Url address, example: "https://www.radaeepdf.com"</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean AddAnnotURI( PDFRect rect, String ^uri )
			{
				char *tmp = cvt_str_cstr( uri );
				bool ret = Page_addAnnotURI2( m_page, (const PDF_RECT *)&rect, tmp );
				free(tmp);
				return ret;
			}
			Boolean AddAnnotPopup(PDFAnnot ^parent, PDFRect rect, bool open);
			/// <summary>
			/// Add a bitmap object as an annotation to page.
			/// You should re - render page to display modified data.
			/// This can be invoked after ObjsStart or Render or RenderToBmp.
			/// This method require professional or premium license, and Document.SetCache() invoked.
			/// </summary>
			/// <param name="img">PDFDocImage object return from Document.NewImage*();</param>
			/// <param name="rect">A PDFRect object to specify the position and size of the image</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean AddAnnotBitmap(PDFDocImage ^img, PDFRect rect)
			{
				return Page_addAnnotBitmap2(m_page, img->m_image, (const PDF_RECT *)&rect);
			}
			/// <summary>
			/// Add a bitmap object as an annotation to page.
			/// You should re - render page to display modified data.
			/// This can be invoked after ObjsStart or Render or RenderToBmp.
			/// This method require professional or premium license, and Document.SetCache() invoked.
			/// </summary>
			/// <param name="img">PDFDocImage object return from Document.NewImage*();</param>
			/// <param name="mat">A PDFMatrix object containing transform and scale information</param>
			/// <param name="has_alpha">Set if the image has a transparency property</param>
			/// <param name="rect">A PDFRect object to specify the position and size of the image</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean AddAnnotBitmap(PDFDocImage ^img, PDFMatrix ^mat, bool has_alpha, PDFRect rect)
			{
				return Page_addAnnotBitmap(m_page, mat->m_mat, img->m_image, (const PDF_RECT *)&rect);
			}
			/// <summary>
			/// Add a RichMedia annotation to page
			/// You should re - render page to display modified data.
			/// This can be invoked after ObjsStart or Render or RenderToBmp
			/// This method require professional or premium license, and Document.SetCache invoked.
			/// </summary>
			/// <param name="path_player">path-name to flash player. example: "/sdcard/VideoPlayer.swf", "/sdcard/AudioPlayer.swf"</param>
			/// <param name="path_content">path-name to RichMedia content. example: "/sdcard/video.mp4", "/sdcard/audio.mp3"</param>
			/// <param name="type">0: Video, 1: Audio, 2: Flash, 3: 3D Video like* .mpg, * .mp4 ... Audio like * .mp3 ...</param>
			/// <param name="img">DocImage object return from Document.NewImage*();</param>
			/// <param name="rect">A PDFRect object describes the position and size of the richmedia annotation</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean AddAnnotRichMedia(String ^path_player, String ^path_content, int type, PDFDocImage ^img, PDFRect rect)
			{
				return Page_addAnnotRichMedia(m_page, path_player, path_content, type, img->m_image, (const PDF_RECT *)&rect);
			}
			/// <summary>
			/// Add hand-writing to page.
			/// You should render page again to display modified data.
			/// This can be invoked after ObjsStart or Render or RenderToBmp.
			/// This method require professional or premium license
			/// </summary>
			/// <param name="ink">PDFInk object to add</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean AddAnnotInk( PDFInk ^ink )
			{
				return Page_addAnnotInk2( m_page, ink->m_ink );
			}
			/// <summary>
			/// Add polygon to page.
			/// You should render page again to display modified data.
			/// This can be invoked after ObjsStart or Render or RenderToBmp.
			/// This method require professional or premium license
			/// </summary>
			/// <param name="path">Must be a closed contour.</param>
			/// <param name="color">Stroke color formated as 0xAARRGGBB.</param>
			/// <param name="fill_color">Fill color, formated as 0xAARRGGBB. if AA == 0, no fill operations, otherwise alpha value is same to stroke color.</param>
			/// <param name="width">Stroke width in PDF coordinate</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean AddAnnotPolygon(PDFPath ^path, unsigned int color, unsigned int fill_color, float width)
			{
				if (!path) return false;
				return Page_addAnnotPolygon(m_page, path->m_path, color, fill_color, width);
			}
			/// <summary>
			/// Add polyline to page
			/// You should re - render page to display modified data.
			/// This can be invoked after ObjsStart or Render or RenderToBmp.
			/// This method require professional or premium license
			/// </summary>
			/// <param name="path">Must be a set of unclosed lines. do not container any move-to operation except the first point in the path.</param>
			/// <param name="color">Stroke color formated as 0xAARRGGBB.</param>
			/// <param name="style1">Style for start point:
			/// 0: None
			/// 1: Arrow
			/// 2: Closed Arrow
			/// 3: Square
			/// 4: Circle
			/// 5: Butt
			/// 6: Diamond
			/// 7: Reverted Arrow
			/// 8: Reverted Closed Arrow
			/// 9: Slash</param>
			/// <param name="style2">Style for end point, values are same as style1.</param>
			/// <param name="fill_color">Fill color, formated as 0xAARRGGBB. if AA == 0, no fill operations, otherwise alpha value is same to stroke color.</param>
			/// <param name="width">Stroke width in PDF coordinate</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean AddAnnotPolyline(PDFPath ^path, unsigned int color, int style1, int style2, unsigned int fill_color, float width)
			{
				if (!path) return false;
				return Page_addAnnotPolyline(m_page, path->m_path, style1, style2, color, fill_color, width);
			}
			/// <summary>
			/// Add line to page.
			/// You should re - render page to display modified data.
			/// This can be invoked after ObjsStart or Render or RenderToBmp.
			/// This method require professional or premium license
			/// </summary>
			/// <param name="x1">x coordinate of start point</param>
			/// <param name="y1">y coordinate of start point</param>
			/// <param name="x2">x coordinate of end point</param>
			/// <param name="y2">y coordinate of end point</param>
			/// <param name="style1">Style for start point:
			/// 0: None
			/// 1: Arrow
			/// 2: Closed Arrow
			/// 3: Square
			/// 4: Circle
			/// 5: Butt
			/// 6: Diamond
			/// 7: Reverted Arrow
			/// 8: Reverted Closed Arrow
			/// 9: Slash</param>
			/// <param name="style2">Style for end point, values are same as style1.</param>
			/// <param name="width">Line width in DIB coordinate</param>
			/// <param name="color">Line color. same as addAnnotRect.</param>
			/// <param name="icolor">Fill color. same as addAnnotRect.</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean AddAnnotLine( float x1, float y1, float x2, float y2, int style1, int style2, float width, unsigned int color, unsigned int icolor )
			{
				PDF_POINT pt1;
				PDF_POINT pt2;
				pt1.x = x1;
				pt1.y = y1;
				pt2.x = x2;
				pt2.y = y2;
				return Page_addAnnotLine2( m_page, &pt1, &pt2, style1, style2, width, color, icolor );
			}
			/// <summary>
			/// Add rectangle to page.
			/// You should render page again to display modified data.
			/// This can be invoked after ObjsStart or Render or RenderToBmp.
			/// This method require professional or premium license
			/// </summary>
			/// <param name="rect">A PDFRect object to describe the position and size of the annotation</param>
			/// <param name="width">Line width in PDF coordinate.</param>
			/// <param name="color">Rectangle color, formated as 0xAARRGGBB</param>
			/// <param name="icolor">Fill color in rectangle, formated as 0xAARRGGBB, if alpha channel is 0, means no fill operation, otherwise alpha channel are ignored.</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean AddAnnotRect( PDFRect rect, float width, unsigned int color, unsigned int icolor )
			{
				return Page_addAnnotRect2( m_page, (const PDF_RECT *)&rect, width, color, icolor );
			}
			/// <summary>
			/// Add ellipse to page.
			/// You should render page again to display modified data.
			/// This can be invoked after ObjsStart or Render or RenderToBmp.
			/// This method require professional or premium license
			/// </summary>
			/// <param name="rect">A PDFRect object to describe the position and size of the annotation</param>
			/// <param name="width">Line width in PDF coordinate.</param>
			/// <param name="color">Rectangle color, formated as 0xAARRGGBB</param>
			/// <param name="icolor">Fill color in rectangle, formated as 0xAARRGGBB, if alpha channel is 0, means no fill operation, otherwise alpha channel are ignored.</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean AddAnnotEllipse( PDFRect rect, float width, unsigned int color, unsigned int icolor )
			{
				return Page_addAnnotEllipse2( m_page, (const PDF_RECT *)&rect, width, color, icolor );
			}
			/// <summary>
			/// Add a sticky text annotation to page.
			/// You should render page again to display modified data.
			/// This can be invoked after ObjsStart or Render or RenderToBmp.
			/// This method require professional or premium license
			/// </summary>
			/// <param name="x">x in PDF coordinate</param>
			/// <param name="y">y in PDF coordinate</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean AddAnnotTextNote( float x, float y )
			{
				return Page_addAnnotText2( m_page, x, y );
			}
			/// <summary>
			/// Add an edit-box on page.
			/// The font of edit box is set by Global.setTextFont in Global.Init().
			/// This can be invoked after ObjsStart or Render or RenderToBmp.
			/// This method require premium license.
			/// </summary>
			/// <param name="rect">A PDFRect object to describe the position and size of the annotation</param>
			/// <param name="line_clr">Color of border line, formated as 0xAARRGGBB.</param>
			/// <param name="line_w">Width of border line.</param>
			/// <param name="fill_clr">Color of background, formated as 0xAARRGGBB. AA must same to line_clr AA, or 0 means no fill color.</param>
			/// <param name="tsize">Text size in DIB coordinate system.</param>
			/// <param name="text_clr">Text color, formated as 0xAARRGGBB. AA must same to line_clr AA</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean AddAnnotEditbox( PDFRect rect, unsigned int line_clr, float line_w, unsigned int fill_clr, float tsize, unsigned int text_clr )
			{
				return Page_addAnnotEditbox2( m_page, (const PDF_RECT *)&rect, line_clr, line_w, fill_clr, tsize, text_clr );
			}
			/// <summary>
			/// Add a file as an attachment to page.
			/// You should render page again to display modified data in viewer.
			/// This can be invoked after ObjsStart or Render or RenderToBmp.
			/// This method require professional or premium license, and Document.SetCache invoked.
			/// </summary>
			/// <param name="rect">A PDFRect object to describe the position and size of the annotation</param>
			/// <param name="path">Absolute path name to the file.</param>
			/// <param name="icon">Icon display to the page. values as:
			/// 0: PushPin
			/// 1 : Graph
			/// 2 : Paperclip
			/// 3 : Tag</param>
			/// <returns></returns>
			Boolean AddAnnotAttachment( PDFRect rect, String ^path, int icon )
			{
				char *tmp = cvt_str_cstr(path);
				bool ret = Page_addAnnotAttachment( m_page, tmp, icon, (const PDF_RECT *)&rect );
				free( tmp );
				return ret;
			}
			/// <summary>
			/// Close page object and free memory.
			/// </summary>
			void Close()
			{
				if (!m_ref && m_page)
					Page_close(m_page);
				m_page = NULL;
				m_ref = false;
			}
			/// <summary>
			/// Advanced function to get reference of annotation object.
			/// This method require premium license.
			/// </summary>
			/// <returns>Reference of the page object</returns>
			PDFRef Advance_GetRef()
			{
				PDF_OBJ_REF ref = Page_advGetRef(m_page);
				PDFRef ret;
				ret.ref = ref;
				return ret;
			}
			/// <summary>
			/// Advanced function to reload annotation object, after advanced methods update annotation object data.
			/// This method require premium license.
			/// </summary>
			void Advance_Reload()
			{
				Page_advReload(m_page);
			}
			/// <summary>
			/// Sign and save the PDF file.
			/// This method required premium license, and signed feature native libs, which has bigger size.
			/// </summary>
			/// <param name="form">Appearance for sign field.</param>
			/// <param name="rect">Rectangle for sign field</param>
			/// <param name="cert_file">A cert file in .p12 or .pfx format, DER encoded cert file.</param>
			/// <param name="pswd">Password to open cert file.</param>
			/// <param name="name">Signer name.</param>
			/// <param name="reason">Sign reason will be written to signature.</param>
			/// <param name="location">Signature location will be written to signature.</param>
			/// <param name="contact">Contact info will write to signature.</param>
			/// <returns>0: OK; other: false</returns>
			int Sign(PDFDocForm ^form, PDFRect rect, String ^cert_file, String ^pswd, String ^name, String ^reason, String ^location, String ^contact)
			{
				char *ccert_file = cvt_str_cstr(cert_file);
				char *cpswd = cvt_str_cstr(pswd);
				char* cname = cvt_str_cstr(name);
				char *creason = cvt_str_cstr(reason);
				char *clocation = cvt_str_cstr(location);
				char *ccontact = cvt_str_cstr(contact);
				return Page_sign(m_page, form->m_form, (const PDF_RECT *)&rect, ccert_file, cpswd, cname, creason, clocation, ccontact);
				free(ccontact);
				free(clocation);
				free(creason);
				free(cname);
				free(cpswd);
				free(ccert_file);
			}
		private:
			PDFPage()
			{
				//m_doc = nullptr;
				m_page = NULL;
				m_ref = false;
			}
			~PDFPage()
			{
				Close();
			}
			friend PDFDoc;
			friend PDFAnnot;
			bool m_ref;
			//PDFDoc ^m_doc;
			PDF_PAGE m_page;
		};
		public ref class PDFAnnot sealed
		{
		public:
			/// <summary>
			/// get annotation type.
			/// This method require professional or premium license
			/// return type as these values :
			/// 0: unknown
			/// 1: text
			/// 2: link
			/// 3: free text
			/// 4: line
			/// 5: square
			/// 6: circle
			/// 7: polygon
			/// 8: polyline
			/// 9: text hilight
			/// 10: text under line
			/// 11: text squiggly
			/// 12: text strikeout
			/// 13: stamp
			/// 14: caret
			/// 15: ink
			/// 16: popup
			/// 17: file attachment
			/// 18: sound
			/// 19: movie
			/// 20: widget
			/// 21: screen
			/// 22: print mark
			/// 23: trap net
			/// 24: water mark
			/// 25: 3d object
			/// 26: rich media
			/// </summary>
			property int Type
			{
				int get(){return Page_getAnnotType(m_page->m_page, m_annot);}
			}
			/// <summary>
			/// Get annotation field flag in acroForm.
			/// This method require premium license
			/// return flag & 1: read - only
			/// flag & 2: is required
			/// flag & 4: no export.
			/// </summary>
			property int FieldType
			{
				int get(){return Page_getAnnotFieldType(m_page->m_page, m_annot);}
			}
			/// <summary>
			/// Get name of the annotation without NO. a fields group with same name "field#0","field#1"��got to "field".
			/// This method require premium license
			/// return null if it is not field, or name of the annotation, example: "EditBox1[0]".
			/// </summary>
			property String ^FieldName
			{
				String ^get()
				{
					wchar_t tmp[512] = {0};
					if( Page_getAnnotFieldNameW(m_page->m_page, m_annot, tmp, 511 ) <= 0 ) return nullptr;
					else return ref new String( tmp );
				}
			}
			/// <summary>
			/// Get name of the annotation.
			/// This method require premium license
			/// return null if it is not field, or name of the annotation, example: "EditBox1[0]".
			/// </summary>
			property String ^FieldNameWithNO
			{
				String ^get()
				{
					wchar_t tmp[512] = { 0 };
					if (Page_getAnnotFieldNameWithNOW(m_page->m_page, m_annot, tmp, 511) <= 0) return nullptr;
					else return ref new String(tmp);
				}
			}
			/// <summary>
			/// Get name of the annotation.
			/// This method require premium license
			/// return null if it is not field, or full name of the annotation, example: "Form1.EditBox1".
			/// </summary>
			property String ^FieldFullName
			{
				String ^get()
				{
					wchar_t tmp[512] = {0};
					if( Page_getAnnotFieldFullNameW(m_page->m_page, m_annot, tmp, 511 ) <= 0 ) return nullptr;
					else return ref new String( tmp );
				}
			}
			/// <summary>
			/// Get full name of the annotation with more details
			/// This method require premium license
			/// return null if it is not field, or full name of the annotation, example: "Form1[0].EditBox1[0]".
			/// </summary>
			property String ^FieldFullName2
			{
				String ^get()
				{
					wchar_t tmp[512] = {0};
					if( Page_getAnnotFieldFullName2W(m_page->m_page, m_annot, tmp, 511 ) <= 0 ) return nullptr;
					else return ref new String( tmp );
				}
			}
			/// <summary>
			/// Get jsvascript action of fields.
			/// This method require premium license.
			/// </summary>
			/// <param name="idx">Action index:
			/// 0:'K' performed when the user types a keystroke
			/// 1: 'F' to be performed before the field is formatted to display its current value.
			/// 2: 'V' to be performed when the field��s value is changed
			/// 3: 'C' to be performed to recalculate the value of this field when that of another field changes.<br / ></param>
			/// <returns>Javsscript of field's action</returns>
			String ^GetFieldJS(int idx)
			{
				return Page_getAnnotFieldJS(m_page->m_page, m_annot, idx);
			}
			/// <summary>
			/// Get/Set if position and size of the annotation is locked?
			/// This method require professional or premium license
			/// </summary>
			property Boolean Locked
			{
				Boolean get(){return Page_isAnnotLocked(m_page->m_page, m_annot);}
				void set(Boolean val) { Page_setAnnotLock(m_page->m_page, m_annot, val); }
			}
			/// <summary>
			/// Check if texts of the annotation is locked?
			/// This method require professional or premium license
			/// </summary>
			property Boolean LockedContent
			{
				Boolean get(){return Page_isAnnotLockedContent(m_page->m_page, m_annot);}
			}
			/// <summary>
			/// Get/Set whether the annotation is hide.
			/// </summary>
			property Boolean Hide
			{
				Boolean get(){return Page_isAnnotHide(m_page->m_page, m_annot);}
				void set(Boolean val){Page_setAnnotHide( m_page->m_page, m_annot, val ); }
			}
			/// <summary>
			/// Get/Set if this annotation is read-only
			/// </summary>
			property Boolean ReadOnly
			{
				Boolean get() { return Page_isAnnotReadOnly(m_page->m_page, m_annot); }
				void set(Boolean val) { Page_setAnnotReadOnly(m_page->m_page, m_annot, val); }
			}
			/// <summary>
			/// Get/Set annotation's box rectangle.
			/// This method require professional or premium license
			/// </summary>
			property PDFRect Rect
			{
				PDFRect get(){PDFRect rect; Page_getAnnotRect(m_page->m_page, m_annot, (PDF_RECT *)&rect); return rect;}
				void set( PDFRect rect ){Page_setAnnotRect( m_page->m_page, m_annot, (const PDF_RECT *)&rect);}
			}
			/// <summary>
			/// Get/Set fill color of square/circle/highlight/line/ploygon/polyline/sticky text/free text/text field annotation.
			/// Page needs to be rendered again to show modified annotation.
			/// This method require professional or premium license
			/// </summary>
			property int FillColor
			{
				int get(){return Page_getAnnotFillColor(m_page->m_page, m_annot);}
				void set( int color ){Page_setAnnotFillColor(m_page->m_page, m_annot, color);}
			}
			/// <summary>
			/// Get/Set stroke color of square/circle/ink/line/underline/Squiggly/strikeout/ploygon/polyline/free text/text field annotation.
			/// Page needs to be rendered again to show modified annotation.
			/// This method require professional or premium license
			/// </summary>
			property int StrokeColor
			{
				int get(){return Page_getAnnotStrokeColor(m_page->m_page, m_annot);}
				void set( int color ){Page_setAnnotStrokeColor(m_page->m_page, m_annot, color);}
			}
			/// <summary>
			/// Get/Set stroke width of square/circle/ink/line/ploygon/polyline/free text/text field annotation
			/// For free text annotation : width of edit - box border
			/// Page needs to be rendered again to show modified annotation.
			/// This method require professional or premium license
			/// </summary>
			property float StrokeWidth
			{
				float get(){return Page_getAnnotStrokeWidth(m_page->m_page, m_annot);}
				void set( float val ){Page_setAnnotStrokeWidth(m_page->m_page, m_annot, val);}
			}
			/// <summary>
			/// Get/Set Path to Ink annotation.
			/// Page needs to be rendered again to show modified annotation.
			/// This method require professional or premium license
			/// </summary>
			property PDFPath ^InkPath
			{
				PDFPath ^get()
				{
					PDF_PATH path = Page_getAnnotInkPath(m_page->m_page, m_annot);
					if (!path) return nullptr;
					return ref new PDFPath(path);
				}
				void set(PDFPath ^path)
				{
					Page_setAnnotInkPath(m_page->m_page, m_annot, path->m_path);
				}
			}
			/// <summary>
			/// Get/Set Path to Polygon annotation.
			/// Page needs to be rendered again to show modified annotation.
			/// This method require professional or premium license
			/// </summary>
			property PDFPath ^PolygonPath
			{
				PDFPath ^get()
				{
					PDF_PATH path = Page_getAnnotPolygonPath(m_page->m_page, m_annot);
					if (!path) return nullptr;
					return ref new PDFPath(path);
				}
				void set(PDFPath ^path)
				{
					Page_setAnnotPolygonPath(m_page->m_page, m_annot, path->m_path);
				}
			}
			/// <summary>
			/// Get/Set Path to Polyline annotation.<br/>
			/// Page needs to be rendered again to show modified annotation.
			/// This method require professional or premium license
			/// </summary>
			property PDFPath ^PolylinePath
			{
				PDFPath ^get()
				{
					PDF_PATH path = Page_getAnnotPolylinePath(m_page->m_page, m_annot);
					if (!path) return nullptr;
					return ref new PDFPath(path);
				}
				void set(PDFPath ^path)
				{
					Page_setAnnotPolylinePath(m_page->m_page, m_annot, path->m_path);
				}
			}
			/// <summary>
			/// Get/Set line style of line or polyline annotation
			/// This method require professional or premium license
			/// return (ret >> 16) is style of end point, (ret & 0xffff) is style of start point.
			/// it has following styles: 
			/// 1: OpenArrow
			/// 2: ClosedArrow
			/// 3: Square
			/// 4: Circle
			/// 5: Butt
			/// 6: Diamond
			/// 7: ROpenArrow
			/// 8: RClosedArrow
			/// 9: Slash
			/// </summary>
			property int LineStyle
			{
				int get() { return Page_getAnnotLineStyle(m_page->m_page, m_annot); }
				void set(int val) { Page_setAnnotLineStyle(m_page->m_page, m_annot, val); }
			}
			/// <summary>
			/// Get/Set icon for sticky text note/file attachment/Rubber Stamp annotation.<br/>
			/// You need render page again to show modified annotation.<br / >
			/// This method require professional or premium license
			/// Available values for sticky text note: 
			/// 0: Note
			/// 1: Comment
			/// 2: Key
			/// 3: Help
			/// 4: NewParagraph
			/// 5: Paragraph
			/// 6: Insert
			/// 7: Check
			/// 8: Circle
			/// 9: Cross
			/// Available values for file attachment: 
			/// 0: PushPin
			/// 1: Graph
			/// 2: Paperclip
			/// 3: Tag
			/// Available values for Rubber Stamp: 
			/// 0: "Draft"(default icon) 
			/// 1: "Approved" 
			/// 2: "Experimental" 
			/// 3: "NotApproved" 
			/// 4: "AsIs" 
			/// 5: "Expired" 
			/// 6: "NotForPublicRelease" 
			/// 7: "Confidential" 
			/// 8: "Final" 
			/// 9: "Sold" 
			/// 10: "Departmental" 
			/// 11: "ForComment" 
			/// 12: "TopSecret" 
			/// 13: "ForPublicRelease" 
			/// 14: "Accepted" 
			/// 15: "Rejected" 
			/// 16: "Witness" 
			/// 17: "InitialHere" 
			/// 18: "SignHere" 
			/// 19: "Void" 
			/// 20: "Completed" 
			/// 21: "PreliminaryResults" 
			/// 22: "InformationOnly" 
			/// 23: "End"
			/// </summary>
			property int Icon
			{
				int get(){return Page_getAnnotIcon(m_page->m_page, m_annot);}
				void set( int icon ){Page_setAnnotIcon(m_page->m_page, m_annot, icon);}
			}
			/// <summary>
			/// Get annotation's destination
			/// This method require professional or premium license
			/// Return 0 based page index number or -1 if failed
			/// </summary>
			property int Dest
			{
				int get(){return Page_getAnnotDest(m_page->m_page, m_annot);}
			}
			/// <summary>
			/// Get annotation's java-script string.
			/// This method require professional or premium license
			/// Return string of javascript, or null.
			/// </summary>
			property String ^JS
			{
				String ^get() {return Page_getAnnotJS(m_page->m_page, m_annot);}
			}
			/// <summary>
			/// Check if the annotation URI annotation
			/// </summary>
			property bool IsURI
			{
				bool get()
				{
					String^ ret = Page_getAnnotURI(m_page->m_page, m_annot);
					return (ret != nullptr && !ret->IsEmpty());
				}
			}
			/// <summary>
			/// Get annotation's URL link string.
			/// This method require professional or premium license
			/// return string of URL, or null
			/// </summary>
			property String ^URI
			{
				String ^get()
				{
					return Page_getAnnotURI(m_page->m_page, m_annot);
				}
			}
			/// <summary>
			/// Check if the annotation file link annotation
			/// </summary>
			property bool IsFileLink
			{
				bool get()
				{
					String ^ret = Page_getAnnotFileLink(m_page->m_page, m_annot);
					return (ret != nullptr && !ret->IsEmpty());
				}
			}
			/// <summary>
			/// Get annotation's file link string.
			/// This method require professional or premium license
			/// return string of URL, or null
			/// </summary>
			property String ^FileLink
			{
				String ^get()
				{
					return Page_getAnnotFileLink(m_page->m_page, m_annot);
				}
			}
			/// <summary>
			/// Check if the annotation remote link annotation
			/// </summary>
			property bool IsRemoteDest
			{
				bool get()
				{
					String^ ret = Page_getAnnotRemoteDest(m_page->m_page, m_annot);
					return (ret != nullptr && !ret->IsEmpty());
				}
			}
			/// <summary>
			/// Get annotation's remote link string.
			/// This method require professional or premium license
			/// return string of URL, or null
			/// </summary>
			property String ^RemoteDest
			{
				String ^get()
				{
					return Page_getAnnotRemoteDest(m_page->m_page, m_annot);
				}
			}
			/// <summary>
			/// Get index of this annotation in page.
			/// return 0 based index value or -1;
			/// </summary>
			property int IndexInPage
			{
				int get()
				{
					int cur = 0;
					int cnt = m_page->AnnotCount;
					while( cur < cnt )
					{
						PDF_ANNOT tmp = ::Page_getAnnot( m_page->m_page, cur );
						if( tmp == m_annot ) return cur;
						cur++;
					}
					return -1;
				}
			}
			/// <summary>
			/// Move annotation to another page.
			/// This method require professional or premium license.
			/// This method just like invoke Page.CopyAnnot() and Annotation.RemoveFromPage(), but, faster and less memory allocateed.
			/// Notice: ObjsStart or RenderXXX shall be invoked for dst_page.
			/// </summary>
			/// <param name="page">Page to move.</param>
			/// <param name="rect">A PDFRect object to describe new position and size of the annotation</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean MoveToPage( PDFPage ^page, PDFRect rect )
			{
				if( !page || !m_page ) return false;
				return Page_moveAnnot( m_page->m_page, page->m_page, m_annot, (const PDF_RECT *)&rect );
			}
			/// <summary>
			/// Remove annotation.
			/// Page should be rendered again after updating.
			/// This method require professional or premium license
			/// </summary>
			/// <returns>True if successed, otherwise false</returns>
			Boolean RemoveFromPage()
			{
				bool ret = Page_removeAnnot( m_page->m_page, m_annot );
				if( ret )
				{
					m_page = nullptr;
					m_annot = NULL;
				}
				return ret;
			}
			/// <summary>
			/// Remove this annotation, and display as normal content in page.
			/// </summary>
			/// <returns>True if successed, otherwise false</returns>
			Boolean FlateFromPage()
			{
				bool ret = Page_flateAnnot(m_page->m_page, m_annot);
				if (ret)
				{
					m_page = nullptr;
					m_annot = NULL;
				}
				return ret;
			}
			/// <summary>
			/// ender page to Bitmap object directly. this function returned for cancelled or finished.
			/// Before render, you need erase Bitmap object.
			/// </summary>
			/// <param name="bmp">Bitmap object to render.</param>
			/// <returns>True if successed, otherwise false</returns>
			Boolean RenderToBmp(PDFBmp ^bmp)
			{
				if (!bmp || !m_page) return false;
				return Page_renderAnnotToBmp(m_page->m_page, m_annot, bmp->m_bmp);
			}
			/// <summary>
			/// Check if this annotation is a movie annotation
			/// </summary>
			property bool IsMovie
			{
				bool get()
				{
					String^ ret = Page_getAnnotMovie(m_page->m_page, m_annot);
					return (ret != nullptr && !ret->IsEmpty());
				}
			}
			/// <summary>
			/// Get annotation's movie name.
			/// This method require professional or premium license
			/// </summary>
			/// <returns>Name of the movie, or null</returns>
			String ^GetMovieName()
			{
				return Page_getAnnotMovie(m_page->m_page, m_annot);
			}
			/// <summary>
			/// Get annotation's movie data, and save to file.
			/// This method require professional or premium license
			/// </summary>
			/// <param name="save_path">Full path name to save data.</param>
			/// <returns>True if save_file created, or false.</returns>
			Boolean GetMovieData( String ^save_path )
			{
				char *tmp = cvt_str_cstr( save_path );
				bool ret = Page_getAnnotMovieData( m_page->m_page, m_annot, tmp );
				free( tmp );
				return ret;
			}
			/// <summary>
			/// Check if this annotation is a 3D annotation
			/// </summary>
			property bool Is3D
			{
				bool get()
				{
					String^ ret = Page_getAnnot3D(m_page->m_page, m_annot);
					return (ret != nullptr && !ret->IsEmpty());
				}
			}
			/// <summary>
			/// Get annotation's 3D component name.
			/// This method require professional or premium license
			/// </summary>
			/// <returns>Name of the 3D component, or null</returns>
			String ^Get3DName()
			{
				return Page_getAnnot3D( m_page->m_page, m_annot);
			}
			/// <summary>
			/// Get annotation's 3D component data, and save to file.
			/// This method require professional or premium license
			/// </summary>
			/// <param name="save_path">Full path name to save data.</param>
			/// <returns>True if save_file created, or false.</returns>
			Boolean Get3DData( String ^save_path )
			{
				char *tmp = cvt_str_cstr( save_path );
				bool ret = Page_getAnnot3DData( m_page->m_page, m_annot, tmp );
				free( tmp );
				return ret;
			}
			/// <summary>
			/// Check if this annotation is an attachment annotation
			/// </summary>
			property bool IsAttachment
			{
				bool get()
				{
					String^ ret = Page_getAnnotAttachment(m_page->m_page, m_annot);
					return (ret != nullptr && !ret->IsEmpty());
				}
			}
			/// <summary>
			/// Get annotation's attachment name.
			/// This method require professional or premium license
			/// </summary>
			/// <returns>Name of the attachment</returns>
			String ^GetAttachmentName()
			{
				return Page_getAnnotAttachment(m_page->m_page, m_annot);
			}
			/// <summary>
			/// Get annotation's attachment data, and save to file.
			/// This method require professional or premium license
			/// </summary>
			/// <param name="save_path">Full path name to save data.</param>
			/// <returns>True if save_file created, or false.</returns>
			Boolean GetAttachmentData( String ^save_path )
			{
				char *tmp = cvt_str_cstr( save_path );
				bool ret = Page_getAnnotAttachmentData( m_page->m_page, m_annot, tmp );
				free( tmp );
				return ret;
			}

			/// <summary>
			/// Get annotation's rendition name.
			/// This method require professional or premium license
			/// </summary>
			/// <returns>Name of the rendition</returns>
			String^ GetRenditionName()
			{
				return Page_getAnnotRendition(m_page->m_page, m_annot);
			}
			/// <summary>
			/// Get annotation's rendition data, and save to file.
			/// This method require professional or premium license
			/// </summary>
			/// <param name="save_path">Full path name to save data.</param>
			/// <returns>True if save_file created, or false.</returns>
			Boolean GetRenditionData(String^ save_path)
			{
				char* tmp = cvt_str_cstr(save_path);
				bool ret = Page_getAnnotRenditionData(m_page->m_page, m_annot, tmp);
				free(tmp);
				return ret;
			}
			/// <summary>
			/// Check if the annotation is a sound annotation
			/// </summary>
			property bool IsSound
			{
				bool get()
				{
					String^ ret = Page_getAnnotSound(m_page->m_page, m_annot);
					return (ret != nullptr && !ret->IsEmpty());
				}
			}
			/// <summary>
			/// Get annotation's sound name.
			/// This method require professional or premium license
			/// </summary>
			/// <returns>Name of the sound</returns>
			String ^GetSoundName()
			{
				return Page_getAnnotSound(m_page->m_page, m_annot);
			}
			/// <summary>
			/// Get annotation's sound data, and save to file.
			/// This method require professional or premium license
			/// </summary>
			/// <param name="save_path">Full path name to save data.</param>
			/// <returns>True if save_file created, or false.</returns>
			Array<int> ^GetSoundData( String ^save_path )
			{
				int paras[6];
				char *tmp = cvt_str_cstr( save_path );
				bool ret = Page_getAnnotSoundData( m_page->m_page, m_annot, paras, tmp );
				free( tmp );
				if( !ret ) return nullptr;
				else
				{
					Array<int> ^tmp = ref new Array<int>(6);
					memcpy( tmp->Data, paras, sizeof(int) * 6 );
					return tmp;
				}
			}
			/// <summary>
			/// Get item count of RichMedia annotation.
			/// This method require professional or premium license.
			/// </summary>
			property int RichMediaItemCount
			{
				int get()
				{
					return Page_getAnnotRichMediaItemCount(m_page->m_page, m_annot);
				}
			}
			/// <summary>
			/// Get actived item of RichMedia annotation.
			/// Return the index of activated richmedia item.
			/// This method require professional or premium license.
			/// </summary>
			property int RichMediaItemActived
			{
				int get()
				{
					return Page_getAnnotRichMediaItemActived(m_page->m_page, m_annot);
				}
			}
			/// <summary>
			/// Get content type of an item of RichMedia annotation.
			/// This method require professional or premium license.
			/// </summary>
			/// <param name="idx">Index of the item, range in [0, Annot.GetRichMediaItemCount()]</param>
			/// <returns>Type of item:
			/// -1: unknown or error.
			/// 0: Video.
			/// 1��Sound.
			/// 2: Flash file object.
			/// 3: 3D file object.</returns>
			int GetRichMediaItemType(int idx)
			{
				return Page_getAnnotRichMediaItemType(m_page->m_page, m_annot, idx);
			}
			/// <summary>
			/// Get asset name of content of an item of RichMedia annotation.
			/// This method require professional or premium license.
			/// </summary>
			/// <param name="idx">Index of the item, range in [0, Annot.GetRichMediaItemCount()]</param>
			/// <returns>Asset name, or null. example: "VideoPlayer.swf"</returns>
			String ^GetRichMediaItemAsset(int idx)
			{
				return Page_getAnnotRichMediaItemAsset(m_page->m_page, m_annot, idx);
			}
			/// <summary>
			/// et parameters of an item of RichMedia annotation.
			/// This method require professional or premium license.
			/// </summary>
			/// <param name="idx">Index of the item, range in [0, Annot.GetRichMediaItemCount()]</param>
			/// <returns>Parameter string, or null.
			/// Example: "source=myvideo.mp4&skin=SkinOverAllNoFullNoCaption.swf&skinAutoHide=true&skinBackgroundColor=0x5F5F5F&skinBackgroundAlpha=0.75&volume=1.00" </returns>
			String ^GetRichMediaItemPara(int idx)
			{
				return Page_getAnnotRichMediaItemPara(m_page->m_page, m_annot, idx);
			}
			/// <summary>
			/// Get source of an item of RichMedia annotation.
			/// This method require professional or premium license.
			/// </summary>
			/// <param name="idx">Index of the item, range in [0, Annot.GetRichMediaItemCount()]</param>
			/// <returns>A parameter string, or null.
			/// example: "source=myvideo.mp4&skin=SkinOverAllNoFullNoCaption.swf&skinAutoHide=true&skinBackgroundColor=0x5F5F5F&skinBackgroundAlpha=0.75&volume=1.00"
			/// In this case, the source is "source=myvideo.mp4", return string is "myvideo.mp4" </returns>
			String ^GetRichMediaItemSource(int idx)
			{
				return Page_getAnnotRichMediaItemSource(m_page->m_page, m_annot, idx);
			}
			/// <summary>
			/// Save source of an item of RichMedia annotation to a file.
			/// This method require professional or premium license.
			/// </summary>
			/// <param name="idx">Index of the item, range in [0, Annot.GetRichMediaItemCount()]</param>
			/// <param name="save_path">Absolute path to save file, like "/home/user/app_data/myvideo.mp4"</param>
			/// <returns>True if the output file is saved successfully, otherwise false</returns>
			Boolean GetRichMediaItemSourceData(int idx, String ^save_path)
			{
				return Page_getAnnotRichMediaItemSourceData(m_page->m_page, m_annot, idx, save_path);
			}
			/// <summary>
			/// Save an asset to a file.
			/// This method require professional or premium license.
			/// </summary>
			/// <param name="name">Asset name in RichMedia assets list.
			/// Example:
			/// GetRichMediaItemAsset(0) return player window named as "VideoPlayer.swf"
			/// GetRichMediaItemPara(0) return "source=myvideo.mp4&skin=SkinOverAllNoFullNoCaption.swf&skinAutoHide=true&skinBackgroundColor=0x5F5F5F&skinBackgroundAlpha=0.75&volume=1.00".
			/// so we has 3 assets in item[0] :
			/// 1."VideoPlayer.swf"
			/// 2."myvideo.mp4"
			/// 3."SkinOverAllNoFullNoCaption.swf" </param>
			/// <param name="save_path">Absolute path to save file, like "/home/user/app_data/myvideo.mp4"</param>
			/// <returns>True if the output file is saved successfully, otherwise false</returns>
			Boolean GetRichMediaData(String ^name, String ^save_path)
			{
				return Page_getAnnotRichMediaData(m_page->m_page, m_annot, name, save_path);
			}
			/// <summary>
			/// Check if the annotation is a popup annotation
			/// </summary>
			property bool IsPopup
			{
				bool get()
				{
					String^ ret = Page_getAnnotPopupSubject(m_page->m_page, m_annot);
					return (ret != nullptr && !ret->IsEmpty());
				}
			}
			/// <summary>
			/// Get popup Annotation associate to this annotation.
			/// </summary>
			property PDFAnnot ^Popup
			{
				PDFAnnot ^get()
				{
					PDFAnnot ^ret = ref new PDFAnnot();
					ret->m_annot = Page_getAnnotPopup(m_page->m_page, m_annot);
					ret->m_page = m_page;
					return ret;
				}
			}
			/// <summary>
			/// Check if a popup annotaion is opened
			/// </summary>
			property Boolean PopupOpen
			{
				Boolean get()
				{
					return Page_getAnnotPopupOpen(m_page->m_page, m_annot);
				}
				void set(Boolean open)
				{
					Page_setAnnotPopupOpen(m_page->m_page, m_annot, open);
				}
			}
			property int ReplyCount
			{
				int get()
				{
					return Page_getAnnotReplyCount(m_page->m_page, m_annot);
				}
			}
			PDFAnnot^ GetReply(int idx)
			{
				PDF_ANNOT annot = Page_getAnnotReply(m_page->m_page, m_annot, idx);
				if (!annot) return nullptr;
				PDFAnnot^ ret = ref new PDFAnnot();
				ret->m_page = m_page;
				ret->m_annot = annot;
				return ret;
			}
			/// <summary>
			/// Get/Set subject to a popup annotation
			/// </summary>
			property String ^PopupSubject
			{
				String ^get()
				{
					return Page_getAnnotPopupSubject(m_page->m_page, m_annot);
				}
				void set( String ^txt )
				{
					Page_setAnnotPopupSubjectW( m_page->m_page, m_annot, txt->Data() );
				}
			}
			/// <summary>
			/// Get/Set content to a popup annotation
			/// </summary>
			property String ^PopupText
			{
				String ^get()
				{
					return Page_getAnnotPopupText(m_page->m_page, m_annot);
				}
				void set( String ^txt )
				{
					Page_setAnnotPopupTextW( m_page->m_page, m_annot, txt->Data() );
				}
			}
			/// <summary>
			/// /// Get/Set label to a popup annotation
			/// </summary>
			property String ^PopupLabel
			{
				String ^get()
				{
					return Page_getAnnotPopupLabel(m_page->m_page, m_annot);
				}
				void set(String ^txt)
				{
					Page_setAnnotPopupLabelW(m_page->m_page, m_annot, txt->Data());
				}
			}
			/// <summary>
			/// Get/Set contents of edit-box.
			/// This method require premium license
			/// </summary>
			property String ^EditText
			{
				String ^get()
				{
					return Page_getAnnotEditText(m_page->m_page, m_annot);
				}
				void set( String ^txt )
				{
					bool ret = Page_setAnnotEditTextW( m_page->m_page, m_annot, txt->Data() );
					ret = 0;
				}
			}
			/// <summary>
			/// Set font of edittext.
			/// The page should be re-rendered to display modified data.
			/// This method require premium license.
			/// </summary>
			/// <param name="font">DocFont object from Document.NewFontCID().</param>
			/// <returns>Ture if succssed, otherwise false</returns>
			bool SetEditFont(PDFDocFont ^font)
			{
				if (!font) return false;
				return Page_setAnnotEditFont(m_page->m_page, m_annot, font->m_font);
			}
			/// <summary>
			/// Sign the empty field and save the PDF file.
			/// If the signature field is not empty(signed), it will return failed.
			/// This method require premium license.
			/// </summary>
			/// <param name="form">Appearance icon for signature</param>
			/// <param name="cert_file">A cert file like .p12 or .pfx file, DER encoded cert file.</param>
			/// <param name="pswd">Password to open cert file.</param>
			/// <param name="name">Signer name.</param>
			/// <param name="reason">Sign reason will be written to signature.</param>
			/// <param name="location">Signature location will be written to signature.</param>
			/// <param name="contact">Contact info will be written to signature.</param>
			/// <returns>0: OK
			/// -1: generate parameters error.
			/// -2: it is not signature field, or field has already signed.
			/// -3: invalid annotation data.
			/// -4: save PDF file failed.
			/// -5: cert file open failed.</returns>
			int SignField(PDFDocForm ^form, String ^cert_file, String ^pswd, String ^name, String ^reason, String ^location, String ^contact)
			{
				char *ccert_file = cvt_str_cstr(cert_file);
				char *cpswd = cvt_str_cstr(pswd);
				char *cname = cvt_str_cstr(name);
				char *creason = cvt_str_cstr(reason);
				char *clocation = cvt_str_cstr(location);
				char *ccontact = cvt_str_cstr(contact);
				return Page_signAnnotField(m_page->m_page, m_annot, form->m_form, ccert_file, cpswd, cname, creason, clocation, ccontact);
				free(ccontact);
				free(clocation);
				free(creason);
				free(cname);
				free(cpswd);
				free(ccert_file);
			}
			/// <summary>
			/// Get type of edit-box.
			/// Returns:
			/// -1: this annotation is not text-box.
			/// 1: normal single line.
			/// 2: password.
			/// 3: MultiLine edit area.
			/// This method require premium license
			/// </summary>
			property int EditType
			{
				int get(){return Page_getAnnotEditType(m_page->m_page, m_annot);}
			}
			/// <summary>
			/// Get position and size of edit-box.
			/// For FreeText annotation, position of edit - box is not the position of annotation.
			/// So this function is needed for edit - box.
			/// This method require premium license
			/// </summary>
			property PDFRect EditTextRect
			{
				PDFRect get()
				{
					PDFRect rect;
					if( !Page_getAnnotEditTextRect( m_page->m_page, m_annot, (PDF_RECT *)&rect ) )
					{
						rect.left = 0;
						rect.top = 0;
						rect.right = 0;
						rect.bottom = 0;
					}
					return rect;
				}
			}
			/// <summary>
			/// Get/Set text size of edit-box and edit field.
			/// This method require premium license
			/// </summary>
			property float EditTextSize
			{
				float get(){return Page_getAnnotEditTextSize(m_page->m_page, m_annot);}
				void set(float val) {Page_setAnnotEditTextSize(m_page->m_page, m_annot, val);}
			}
			/// <summary>
			/// Get/Set text align of edit-box and edit field.
			/// Available values:
			/// 0: left, 1: center, 2: right.
			/// This method require premium license
			/// </summary>
			property int EditTextAlign
			{
				int get() { return Page_getAnnotEditTextAlign(m_page->m_page, m_annot); }
				void set(int val) { Page_setAnnotEditTextAlign(m_page->m_page, m_annot, val); }
			}
			/// <summary>
			/// Get/Set text color for edit-box annotation.include text field and free-text.
			/// This method require premium license
			/// </summary>
			property unsigned int EditTextColor
			{
				unsigned int get() {return Page_getAnnotEditTextColor(m_page->m_page, m_annot);}
				void set(unsigned int val) {Page_setAnnotEditTextColor(m_page->m_page, m_annot, val);}
			}
			/// <summary>
			/// Get item count of combo-box.
			/// Returns -1 if the annotation is not a combo-box.
			/// This method require premium license
			/// </summary>
			property int ComboItemCount
			{
				int get(){return Page_getAnnotComboItemCount(m_page->m_page, m_annot);}
			}
			/// <summary>
			/// Get/Set current selected item index of combo-box.
			/// Get() returns -1 if this is not combo-box or no item selected, otherwise the item index that selected.
			/// This method require premium license
			/// </summary>
			property int ComboItemSel
			{
				int get(){return Page_getAnnotComboItemSel(m_page->m_page, m_annot);}
				void set(int item){Page_setAnnotComboItem(m_page->m_page, m_annot, item);}
			}
			/// <summary>
			/// Get export value of combo-box item.
			/// This method require premium license
			/// </summary>
			/// <param name="item">0 based item index. range:[0, GetAnnotComboItemCount()-1]</param>
			/// <returns>null if this is not combo-box or no item with specified index, otherwise the value of the item.</returns>
			String ^GetComboItem( int item )
			{
				return Page_getAnnotComboItem(m_page->m_page, m_annot, item);
			}
			/// <summary>
			/// Get item count of list-box.
			/// This method require premium license
			/// </summary>
			property int ListItemCount
			{
				int get(){return Page_getAnnotListItemCount(m_page->m_page, m_annot);}
			}
			/// <summary>
			/// Get/Set the index of selected list item
			/// This method require premium license
			/// </summary>
			property Array<int> ^ListItemSel
			{
				Array<int> ^get()
				{
					int sels[128];
					int cnt = Page_getAnnotListSels( m_page->m_page, m_annot, sels, 128 );
					Array<int> ^tmp = ref new Array<int>(cnt);
					memcpy( tmp->Data, sels, cnt * sizeof(int) );
					return tmp;
				}
				void set(const Array<int> ^sel)
				{
					Page_setAnnotListSels(m_page->m_page, m_annot, sel->Data, sel->Length);
				}
			}
			/// <summary>
			/// Get an item of list-box.
			/// This method require premium license
			/// </summary>0 based item index. range:[0, GetListItemCount()-1]
			/// <param name="item"></param>
			/// <returns></returns>
			String ^GetListItem( int item )
			{
				return Page_getAnnotListItem(m_page->m_page, m_annot, item);
			}
			/// <summary>
			/// Get status of check-box and radio-box.
			/// This method require premium license
			/// </summary>
			/// <returns>
			/// -1 if annotation is not valid control.
			/// 0 if check-box is not checked.
			/// 1 if check-box checked.
			/// 2 if radio-box is not checked.
			/// 3 if radio-box checked.</returns>
			int GetCheckStatus()
			{
				return Page_getAnnotCheckStatus( m_page->m_page, m_annot );
			}
			/// <summary>
			/// Set value to check-box.
			/// Page should be re-rendered to display modified data.
			/// This method require premium license
			/// </summary>
			/// <param name="check">Indicate if the check-box is checked</param>
			/// <returns>True is successed, otherwise false</returns>
			Boolean SetCheckValue(Boolean check)
			{
				return Page_setAnnotCheckValue( m_page->m_page, m_annot, check );
			}
			/// <summary>
			/// Check the radio-box and clear others in radio group.
			/// The page should be re-rendered to display modified data.
			/// This method require premium license
			/// </summary>
			/// <returns>True is successed, otherwise false</returns>
			Boolean SetRadio()
			{
				return Page_setAnnotRadio( m_page->m_page, m_annot );
			}
			/// <summary>
			/// Get/Set stroke dash of square/circle/ink/line/ploygon/polyline/free text/text field annotation.
			/// For free text or text field annotation : dash of edit - box border
			/// The page should be re-rendered to display modified data.
			/// This method require professional or premium license
			/// </summary>
			property Array<float> ^StrokeDash
			{
				Array<float>^ get() {
					float stmp[128];
					int cnt = Page_getAnnotStrokeDash(m_page->m_page, m_annot, stmp, 128);
					if (cnt <= 0) return nullptr;
					return ref new Array<float>(stmp, cnt);
				}
				void set(const Array<float>^ val)
				{
					if (val && val->Length > 0)
						Page_setAnnotStrokeDash(m_page->m_page, m_annot, val->Data, val->Length);
					else
						Page_setAnnotStrokeDash(m_page->m_page, m_annot, NULL, 0);
				}
			}
			/// <summary>
			/// Check if the annotation is a reset button
			/// </summary>
			/// <returns>True if it is a reset button, otherwise false</returns>
			Boolean IsResetButton()
			{
				return Page_getAnnotReset( m_page->m_page, m_annot );
			}
			/// <summary>
			/// Do annotation reset
			/// </summary>
			/// <returns>True is successed, otherwise false</returns>
			Boolean DoReset()
			{
				return Page_setAnnotReset( m_page->m_page, m_annot );
			}
			/// <summary>
			/// Get annotation submit target on a submit button.
			/// This method require premium license
			/// </summary>
			property String ^SubmitTarget
			{
				String ^get()
				{
					return Page_getAnnotSubmitTarget(m_page->m_page, m_annot);
				}
			}
			/// <summary>
			/// Get annotation submit parameters on a submit button.
			/// Mail mode : return whole XML string for form data.
			/// Other mode : url data likes : "para1=xxx&para2=xxx".
			/// This method require premium license
			/// </summary>
			property String ^SubmitPara
			{
				String ^get()
				{
					wchar_t uri[512];
					if( !Page_getAnnotSubmitParaW( m_page->m_page, m_annot, uri, 511 ) ) return nullptr;
					else return ref new String(uri);
				}
			}
			/// <summary>
			/// Get status of signature field. 
			/// Returns:
			/// -1 if this is not signature field
			/// 0 if not signed.
			/// 1 if signed.
			/// This method require premium license
			/// </summary>
			property int SignStatus
			{
				int get()
				{
					return Page_getAnnotSignStatus(m_page->m_page, m_annot);
				}
			}
			/// <summary>
			/// Get PDFSign object embedded
			/// </summary>
			property PDFSign ^Sign
			{
				PDFSign ^get()
				{
					PDF_SIGN sign = Page_getAnnotSign(m_page->m_page, m_annot);
					if (!sign) return nullptr;
					PDFSign ^ret = ref new PDFSign;
					ret->m_sign = sign;
					return ret;
				}
			}
			/// <summary>
			/// Get annotation reference
			/// </summary>
			property PDFRef Ref
			{
				PDFRef get()
				{
					PDFRef ref;
					ref.ref = Page_getAnnotRef(m_page->m_page, m_annot);
					return ref;
				}
			}
			/// <summary>
			/// Get advanced annotation reference
			/// </summary>
			/// <returns></returns>
			PDFRef Advance_GetRef()
			{
				PDF_OBJ_REF ref = Page_advGetAnnotRef(m_page->m_page, m_annot);
				PDFRef ret;
				ret.ref = ref;
				return ret;
			}
			/// <summary>
			/// Advanced function to reload page object, after advanced methods update Page object data.
			/// All annotations return from Page.GetAnnot() or Page.GetAnnotFromPoint() shall not available.after this method invoked.
			/// This method require premium license.
			/// </summary>
			void Advance_Reload()
			{
				Page_advReloadAnnot(m_page->m_page, m_annot);
			}
			/// <summary>
			/// Export data from annotation.
			/// A premium license is required for this method.
			/// </summary>
			/// <returns>A byte array saved annotation data.</returns>
			Array<BYTE> ^Export()
			{
				unsigned char *buf = (unsigned char *)malloc(8192);
				int len = Page_exportAnnot(m_page->m_page, m_annot, buf, 8192);
				Array<BYTE> ^ret = ref new Array<BYTE>(len);
				memcpy(ret->Data, buf, len);
				free(buf);
				return ret;
			}
			/// <summary>
			/// Get point from line annotation.
			/// This method require professional or premium license
			/// </summary>
			/// <param name="index">0: start point, others: end point.</param>
			/// <returns>The target point</returns>
			PDFPoint GetLinePoint(int index)
			{
				PDF_POINT pt;
				pt.x = 0;
				pt.y = 0;
				Page_getAnnotLinePoint(m_page->m_page, m_annot, index, &pt);
				return *(PDFPoint *)&pt;
			}
			/// <summary>
			/// Set the start and end point of a line annotation
			/// </summary>
			/// <param name="x1">x coordinate of start point</param>
			/// <param name="y1">y coordinate of end point</param>
			/// <param name="x2">x coordinate of start point</param>
			/// <param name="y2">y coordinate of end point</param>
			/// <returns></returns>
			bool SetLinePoint(float x1, float y1, float x2, float y2)
			{
				return Page_setAnnotLinePoint(m_page->m_page, m_annot, x1, y1, x2, y2);
			}
		private:
			PDFAnnot()
			{
				m_page = nullptr;
				m_annot = NULL;
			}
			friend PDFPage;
			PDFPage ^m_page;
			PDF_ANNOT m_annot;
		};
	}
}
