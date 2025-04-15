#pragma once
namespace RDPDFLib
{
	namespace pdf
	{
		ref class PDFMatrix;
		ref class PDFDoc;
	}
	namespace view
	{
		class DXBlock;
		class DXThread;
		class DXDevice;
		class DXPage
		{
		public:
			void dx_init(pdf::PDFDoc ^doc, int pageno)
			{
				m_doc = doc;
				m_pageno = pageno;
				m_left = 0;
				m_top = 0;
				m_right = 0;
				m_bottom = 0;
				m_pw = 0;
				m_ph = 0;
				m_scale = 0;
				m_blks = NULL;
				m_blks_cnt = 0;
				m_blks_zoom = NULL;
				m_blks_zoom_cnt = 0;
				m_dirty = false;
			}
			bool dx_layout(int x, int y, float scale);
			//void dx_layout(int w, int h);
			void dx_alloc();
			inline void dx_set_dirty() {m_dirty = true;}
			void dx_render(DXThread &thread);
			void dx_draw(DXThread &thread, DXDevice *device, int orgx, int orgy, int w, int h);
			void dx_end(DXThread &thread);
			void dx_end_zoom(DXThread &thread);
			void dx_zoom_start(DXThread &thread);

			inline int GetPageNo() { return m_pageno; }
			inline int GetLeft() { return m_left; }
			inline int GetTop() { return m_top; }
			inline int GetRight() { return m_right; }
			inline int GetBottom() { return m_bottom; }
			inline int GetWidth() { return m_right - m_left; }
			inline int GetHeight() { return m_bottom - m_top; }
			inline float GetPDFX(int vx)
			{
				return (vx - ((m_right + m_left - m_pw) >> 1)) / m_scale;
			}
			inline float GetPDFY(int vy)
			{
				return (((m_bottom + m_top + m_ph) >> 1) - vy) / m_scale;
			}
			/**
			 * map x position in view to PDF coordinate
			 * @param x x position in view
			 * @param scrollx x scroll position
			 * @return
			 */
			inline float ToPDFX(float x, float scrollx)
			{
				return (x + scrollx - m_left) / m_scale;
			}
			/**
			 * map y position in view to PDF coordinate
			 * @param y y position in view
			 * @param scrolly y scroll position
			 * @return
			 */
			inline float ToPDFY(float y, float scrolly)
			{
				return (m_bottom - y - scrolly) / m_scale;
			}
			/**
			 * map x to DIB coordinate
			 * @param x x position in PDF coordinate
			 * @return
			 */
			inline float ToDIBX(float x) { return x * m_scale; }
			/**
			 * map y to DIB coordinate
			 * @param y y position in PDF coordinate
			 * @return
			 */
			inline float ToDIBY(float y) { return m_ph - y * m_scale; }
			inline int GetVX(float pdfx)
			{
				return ((m_right + m_left - m_pw) >> 1) + (int)(pdfx * m_scale);
			}
			inline int GetVY(float pdfy)
			{
				return ((m_bottom + m_top + m_ph) >> 1) - (int)(pdfy * m_scale);
			}
			inline float GetScale() { return m_scale; }
			inline float ToPDFSize(float val) { return val / m_scale; }
			pdf::PDFMatrix ^CreateInvertMatrix(float scrollx, float scrolly);
		private:
			pdf::PDFDoc ^m_doc;
			int m_pageno;
			int m_left;
			int m_top;
			int m_right;
			int m_bottom;
			int m_pw;
			int m_ph;
			float m_scale;
			bool m_dirty;
			DXBlock **m_blks;
			int m_blks_cnt;
			DXBlock **m_blks_zoom;
			int m_blks_zoom_cnt;
		};
	}
}
