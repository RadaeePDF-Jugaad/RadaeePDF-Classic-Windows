#pragma once
#include "PDFCore.h"
#include "RDVCallback.h"
#include <ppltasks.h>
//extern int g_iblks;
using namespace Concurrency;
using namespace RDPDFLib::pdf;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::Foundation;
namespace RDPDFLib
{
	namespace view
	{
		class CRDVBlk
		{
		public:
			int m_x;
			int m_y;
			int m_w;
			int m_h;
			double m_ph;
			int m_status;
			int m_pageno;
			double m_scale;
			PDF_DOC m_doc;
			PDF_PAGE m_page;
			PDF_DIB m_dib;
			Image^ m_img;
			WriteableBitmap ^m_bmp;
			bool m_attached;
			static PDFBmp^ m_def_bmp;
			static int m_cell_size;
			CRDVBlk(PDFDoc^ doc, int pageno, double scale, int x, int y, int w, int h, double ph)
			{
				if (!m_def_bmp)
				{
					m_def_bmp = ref new PDFBmp(2, 2);
					m_def_bmp->Reset(-1);
				}
				m_x = x;
				m_y = y;
				m_w = w;
				m_h = h;
				m_ph = ph;
				m_scale = scale;
				m_status = 0;
				m_page = NULL;
				m_dib = NULL;
				m_img = nullptr;
				m_bmp = nullptr;
				m_doc = (PDF_DOC)doc->Handler;
				m_pageno = pageno;
				m_attached = false;
			}
			CRDVBlk(const CRDVBlk* src)
			{
				if (!m_def_bmp)
				{
					m_def_bmp = ref new PDFBmp(2, 2);
					m_def_bmp->Reset(-1);
				}
				m_x = src->m_x;
				m_y = src->m_y;
				m_w = src->m_w;
				m_h = src->m_h;
				m_ph = src->m_ph;
				m_scale = src->m_scale;
				m_status = 0;
				m_page = NULL;
				m_dib = NULL;
				m_img = nullptr;
				m_bmp = nullptr;
				m_doc = src->m_doc;
				m_pageno = src->m_pageno;
				m_attached = false;
			}
			~CRDVBlk()
			{
				if (m_dib)
				{
					Global_dibFree(m_dib);
					m_dib = NULL;
				}
				if (m_page)
				{
					Page_close(m_page);
					m_page = NULL;
				}
				m_doc = NULL;
			}
			inline void bk_render()
			{
				if (m_status == -1) return;
				m_page = Document_getPage(m_doc, m_pageno);
				PDF_PAGE page = m_page;
				PDF_DIB dib = Global_dibGet(NULL, m_w, m_h);
				Page_renderPrepare(page, dib);
				if (m_status == -1)
				{
					Global_dibFree(dib);
					return;
				}
				PDF_MATRIX mat = Matrix_createScale(m_scale, -m_scale, -m_x, m_ph - m_y);
				Page_render(page, dib, mat, true, mode_best);
				Matrix_destroy(mat);

				if (m_status != -1)
				{
					m_dib = dib;
					m_status = 2;
				}
				else
					Global_dibFree(dib);
			}
			inline bool ui_start()
			{
				if (m_status != 0) return false;
				m_status = 1;
				return true;
			}
			inline bool ui_end(IVCallback^ canvas)
			{
				if (m_status == 0 || m_status == -1) return false;
				if (m_status == 1 && m_page) Page_renderCancel(m_page);
				m_status = -1;
				ui_cancel(canvas);
				return true;
			}
			inline void ui_cancel(IVCallback^ canvas)
			{
				if (m_img)
				{
					m_img->Source = nullptr;//m_def_bmp->Data;
					canvas->vpRemoveBlock(m_img);
					m_img = nullptr;
				}
				//seems no help to invoke System.GC.Collect()
				if (m_bmp)
				{
					canvas->vpDetachBmp(m_bmp = nullptr);
				}
			}
			inline bool ui_is_cancel()
			{
				return m_status < 0;
			}
			inline void ui_draw(IVCallback^ canvas, double orgx, double orgy)
			{
				if (!m_img)
				{
					m_img = ref new Image();
					m_img->Stretch = Stretch::Fill;
					m_img->Source = m_def_bmp->Data;
					canvas->vpShowBlock(m_img, orgx + m_x, orgy + m_y, m_w, m_h);
				}
				//else canvas->vpMoveBlock(m_img, left, top, right, bottom);
				if (m_status == 2 && !m_attached)
				{
					if (m_dib)
					{
						m_bmp = ref new WriteableBitmap(m_w, m_h);
						ArrayReference<BYTE> data((BYTE*)Global_dibGetData(m_dib), m_w * m_h * 4);
						canvas->vpAttachBmp(m_bmp, data);
						m_img->Source = m_bmp;

						Global_dibFree(m_dib);
						m_dib = NULL;
					}
					m_attached = true;
				}
			}
			inline void ui_draw(IVCallback^ canvas, int orgx, int orgy)
			{
				if (!m_img)
				{
					m_img = ref new Image();
					m_img->Stretch = Stretch::Fill;
					m_img->Source = m_def_bmp->Data;
					canvas->vpShowBlock(m_img, orgx + m_x, orgy + m_y, m_w, m_h);
				}
				//else canvas->vpMoveBlock(m_img, left, top, right, bottom);
				if (m_status == 2 && !m_attached)
				{
					if (m_dib)
					{
						m_bmp = ref new WriteableBitmap(m_w, m_h);
						ArrayReference<BYTE> data((BYTE*)Global_dibGetData(m_dib), m_w * m_h * 4);
						canvas->vpAttachBmp(m_bmp, data);
						m_img->Source = m_bmp;

						Global_dibFree(m_dib);
						m_dib = NULL;
					}
					m_attached = true;
				}
			}
			inline bool isCross(double left, double top, double right, double bottom)
			{
				return !(left >= m_x + m_w || right < m_x || top >= m_y + m_h || bottom < m_y);
			}
			inline bool isRender()
			{
				return (m_status == 1 || m_status == 2);
			}
			inline bool isFinished()
			{
				return (m_status == 2);
			}
			inline double GetX()
			{
				return m_x;
			}
			inline double GetY()
			{
				return m_y;
			}
			inline double GetRight()
			{
				return m_x + m_w;
			}
			inline double GetBottom()
			{
				return m_y + m_h;
			}
			inline double GetW()
			{
				return m_w;
			}
			inline double GetH()
			{
				return m_h;
			}
			inline int GetPageNo() { return m_pageno; }
		};
	}
}
