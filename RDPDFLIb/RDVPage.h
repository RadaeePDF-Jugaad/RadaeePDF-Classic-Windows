#pragma once
#include "PDFCore.h"
#include "RDVThread.h"
#include "RDVBlk.h"
#include "RDVCallback.h"

namespace RDPDFLib
{
	namespace view
	{
		static void utf16_cpy(wchar_t* sDst, const wchar_t* sSrc)
		{
			if (!sDst || !sSrc) return;
			while (*sDst++ = *sSrc++);
		}
		static void utf16_from_i32(wchar_t *sVal, int iVal)
		{
			if (iVal == 0)
			{
				sVal[0] = '0';
				sVal[1] = 0;
				return;
			}
			wchar_t tmp[12] = { 0 };
			unsigned dwOff = 10;
			unsigned dwVal;
			if (iVal < 0)
				dwVal = -iVal;
			else
				dwVal = iVal;
			tmp[dwOff] = '0';
			for (dwOff = 10; dwVal; dwOff--)
			{
				tmp[dwOff] = '0' + dwVal % 10;
				dwVal /= 10;
			}
			dwOff++;
			if (iVal < 0)
			{
				dwOff--;
				tmp[dwOff] = '-';
			}
			utf16_cpy(sVal, tmp + dwOff);
		}
		class CRDVPage
		{
		public:
			PDFDoc ^m_doc;
			int m_pageno;
			double m_left;
			double m_top;
			double m_right;
			double m_bottom;
			double m_pw;
			double m_ph;
			double m_scale;
			bool m_dirty;
			CRDVBlk** m_blks;
			int m_blks_cnt;
			CRDVBlk** m_blks_zoom;
			int m_blks_zoom_cnt;
			TextBlock^ m_pno;
			void free_blks()
			{
				//CRDVBlk** cur = m_blks;
				//CRDVBlk** end = cur + m_blks_cnt;
				//while (cur < end)
				//	delete* cur++;
				free(m_blks);
				m_blks = NULL;
				m_blks_cnt = 0;
			}
			void free_blks_zoom()
			{
				//CRDVBlk** cur = m_blks_zoom;
				//CRDVBlk** end = cur + m_blks_zoom_cnt;
				//while (cur < end)
				//	delete* cur++;
				free(m_blks_zoom);
				m_blks_zoom = NULL;
				m_blks_zoom_cnt = 0;
			}
			CRDVPage()
			{
				m_doc = nullptr;
				m_pageno = 0;
				m_blks = NULL;
				m_blks_cnt = 0;
				m_blks_zoom = NULL;
				m_blks_zoom_cnt = 0;
				m_dirty = false;
				m_scale = -1;
				m_pno = nullptr;
				m_dirty = false;
				m_left = 0;
				m_top = 0;
				m_right = 0;
				m_bottom = 0;
				m_pw = 0;
				m_ph = 0;
			}
			inline void init(PDFDoc ^doc, int pageno)
			{
				m_doc = doc;
				m_pageno = pageno;
				m_blks = NULL;
				m_blks_cnt = 0;
				m_blks_zoom = NULL;
				m_blks_zoom_cnt = 0;
				m_dirty = false;
				m_scale = -1;
				m_left = 0;
				m_top = 0;
				m_right = 0;
				m_bottom = 0;
			}
			inline void ui_layout(double x, double y, double scale)
			{
				m_left = x;
				m_top = y;
				m_pw = scale * m_doc->GetPageWidth(m_pageno);
				m_ph = scale * m_doc->GetPageHeight(m_pageno);
				m_right = m_left + m_pw;
				m_bottom = m_top + m_ph;
				m_scale = scale;
				if (!m_blks) ui_alloc();
			}
			inline void ui_alloc()
			{
				double width = m_right - m_left;
				double height = m_bottom - m_top;
				int csize2 = CRDVBlk::m_cell_size << 1;
				if (CRDVBlk::m_cell_size > 0 && (width >= csize2 || height >= csize2))
				{
					int xcnt = (width + CRDVBlk::m_cell_size - 1) / CRDVBlk::m_cell_size;
					int ycnt = (height + CRDVBlk::m_cell_size - 1) / CRDVBlk::m_cell_size;
					m_blks_cnt = xcnt * ycnt;
					m_blks = (CRDVBlk**)calloc(m_blks_cnt, sizeof(CRDVBlk*));
					int cury = 0;
					int ext = 1;
					for (int yb = 0; yb < ycnt; yb++)
					{
						int curx = 0;
						int bheight = (yb < ycnt - 1) ? CRDVBlk::m_cell_size : height - cury + 1;
						for (int xb = 0; xb < xcnt; xb++)
						{
							int bwidth = (xb < xcnt - 1) ? CRDVBlk::m_cell_size : width - curx + 1;
							m_blks[yb * xcnt + xb] = new CRDVBlk(m_doc, m_pageno, m_scale, curx, cury, bwidth + ext, bheight + ext, height);
							curx += bwidth;
						}
						cury += bheight;
					}
				}
				else
				{
					m_blks_cnt = 1;
					m_blks = (CRDVBlk**)malloc(sizeof(CRDVBlk*));
					m_blks[0] = new CRDVBlk(m_doc, m_pageno, m_scale, 0, 0, m_right - m_left, m_bottom - m_top, height);
				}
			}
			inline void ui_set_dirty()
			{
				m_dirty = true;
			}
			inline void ui_draw_pno(IVCallback^ canvas)
			{
				if (m_pno == nullptr)
				{
					wchar_t wtxt[16];
					utf16_from_i32(wtxt, m_pageno + 1);
					m_pno = ref new TextBlock();
					m_pno->Text = ref new String(wtxt);
					m_pno->IsHitTestVisible = false;
					m_pno->TextAlignment = Windows::UI::Xaml::TextAlignment::Center;
					m_pno->FontSize = 24;
					Windows::UI::Color clr;
					clr.A = 160;
					clr.B = 255;
					clr.G = 0;
					clr.R = 0;
					m_pno->Foreground = ref new SolidColorBrush(clr);
				}
				canvas->vpShowPNO(m_pno, m_left, m_top, m_right, m_bottom);
			}
			inline void ui_end_pno(IVCallback^ canvas)
			{
				if (m_pno)
				{
					canvas->vpRemovePNO(m_pno);
					m_pno = nullptr;
				}
			}
			inline void ui_draw(IVCallback^canvas, CRDVThread& thread, int orgx, int orgy, int w, int h)
			{
				if (m_dirty)
				{
					m_dirty = false;
					ui_zoom_start(canvas, thread);
					ui_alloc();
				}
				int bl = m_left - orgx;
				int bt = m_top - orgy;
				int br = m_right - orgx;
				int bb = m_bottom - orgy;
				if (bl < 0) bl = 0;
				if (bt < 0) bt = 0;
				if (br > w) br = w;
				if (bb > h) bb = h;
				if (br > bl&& bb > bt)//draw white
				{
					//todo:draw white
				}
				if (m_blks_zoom)//draw zoom cache first.
				{
					CRDVBlk* glb = m_blks_zoom[m_blks_zoom_cnt - 1];
					double srcw = glb->GetRight();
					double srch = glb->GetBottom();
					double dstw = m_right - m_left;
					double dsth = m_bottom - m_top;
					for (int ibb = 0; ibb < m_blks_zoom_cnt; ibb++) {
						glb = m_blks_zoom[ibb];
						bl = m_left + glb->m_x * dstw / srcw;
						bt = m_top + glb->m_y * dsth / srch;
						br = m_left + glb->GetRight() * dstw / srcw;
						bb = m_top + glb->GetBottom() * dsth / srch;
						if (br <= 0 || bl >= w || bb <= 0 || bt >= h) continue;
						glb->ui_draw(canvas, m_left, m_top);
					}
				}
				if (!m_blks) return;//then draw the blocks.
				bool all_ok = true;
				int iorgx = m_left + 0.5;//using int offset x can render images more clear(UWP issue)
				int iorgy = m_top + 0.5;//using int offset y can render images more clear(UWP issue)
				int left = m_left - orgx;
				int top = m_top - orgy;
				bl = -left - CRDVBlk::m_cell_size;
				bt = -top - CRDVBlk::m_cell_size;
				br = w - left + CRDVBlk::m_cell_size;
				bb = h - top + CRDVBlk::m_cell_size;
				for (int ib = 0; ib < m_blks_cnt; ib++)
				{
					CRDVBlk* glb = m_blks[ib];
					if (glb->isCross(bl, bt, br, bb))
					{
						if (!glb->isFinished())
						{
							all_ok = false;
							thread.render_start(glb);
							if (!m_blks_zoom)//not in zooming status.
								glb->ui_draw(canvas, iorgx, iorgy);
						}
						else//texture is ready
							glb->ui_draw(canvas, iorgx, iorgy);
					}
					else if(glb->isRender())
					{
						m_blks[ib] = thread.render_end2(canvas, glb);
					}
				}
				if (all_ok) ui_end_zoom(canvas, thread);//destroy zoom cache and draw blk.
			}
			inline void ui_reset(IVCallback^ canvas, CRDVThread& thread)
			{
				if (!m_blks) return;
				for (int cur = 0; cur < m_blks_cnt; cur++)
				{
					CRDVBlk* glb = m_blks[cur];
					m_blks[cur] = thread.render_end2(canvas, glb);
				}
			}
			inline void ui_end(IVCallback^ canvas, CRDVThread& thread)
			{
				if (!m_blks) return;
				for (int cur = 0; cur < m_blks_cnt; cur++)
				{
					CRDVBlk* glb = m_blks[cur];
					if (!thread.render_end(canvas, glb))
					{
						if (!glb->ui_is_cancel())
							delete glb;
						else
							glb->ui_cancel(canvas);
					}
				}
				free_blks();
			}
			inline void ui_end_zoom(IVCallback^ canvas, CRDVThread& thread)
			{
				if (!m_blks_zoom) return;
				for (int cur = 0; cur < m_blks_zoom_cnt; cur++)
				{
					CRDVBlk* glb = m_blks_zoom[cur];
					if (!thread.render_end(canvas, glb))
					{
						if (!glb->ui_is_cancel())
							delete glb;
						else
							glb->ui_cancel(canvas);
					}
				}
				free_blks_zoom();
			}
			inline void ui_zoom_start(IVCallback^ canvas, CRDVThread& thread)
			{
				if (m_blks_zoom)
				{
					if (m_blks)
					{
						for (int cur = 0; cur < m_blks_cnt; cur++)
						{
							CRDVBlk* glb = m_blks[cur];
							if (!thread.render_end(canvas, glb))
							{
								if (!glb->ui_is_cancel())
									delete glb;
								else
									glb->ui_cancel(canvas);
							}
						}
						free_blks();
					}
					return;
				}
				m_blks_zoom = m_blks;
				m_blks_zoom_cnt = m_blks_cnt;
				m_blks = NULL;
				m_blks_cnt = 0;
			}
			inline int GetPageNo() { return m_pageno; }
			inline int GetLeft() { return m_left; }
			inline int GetTop() { return m_top; }
			inline int GetRight() { return m_right; }
			inline int GetBottom() { return m_bottom; }
			inline int GetWidth() { return m_right - m_left; }
			inline int GetHeight() { return m_bottom - m_top; }
			inline double GetPDFX(double vx)
			{
				return (vx - ((m_right + m_left - m_pw) * 0.5)) / m_scale;
			}
			inline double GetPDFY(double vy)
			{
				return (((m_bottom + m_top + m_ph) * 0.5) - vy) / m_scale;
			}
			/**
			 * map x position in view to PDF coordinate
			 * @param x x position in view
			 * @param scrollx x scroll position
			 * @return
			 */
			inline double ToPDFX(double x, double scrollx)
			{
				return (x + scrollx - m_left) / m_scale;
			}
			/**
			 * map y position in view to PDF coordinate
			 * @param y y position in view
			 * @param scrolly y scroll position
			 * @return
			 */
			inline double ToPDFY(double y, double scrolly)
			{
				return (m_bottom - y - scrolly) / m_scale;
			}
			/**
			 * map x to DIB coordinate
			 * @param x x position in PDF coordinate
			 * @return
			 */
			inline double ToDIBX(double x) { return m_scale * x; }
			/**
			 * map y to DIB coordinate
			 * @param y y position in PDF coordinate
			 * @return
			 */
			inline double ToDIBY(double y) { return double(m_ph) - m_scale * y; }
			inline double GetVX(double pdfx)
			{
				return ((m_right + m_left - m_pw) * 0.5) + (pdfx * m_scale);
			}
			inline double GetVY(double pdfy)
			{
				return ((m_bottom + m_top + m_ph) * 0.5) - (pdfy * m_scale);
			}
			inline double GetScale() { return m_scale; }
			inline double ToPDFSize(double val) { return val / m_scale; }
			inline PDFMatrix ^CreateInvertMatrix(double scrollx, double scrolly)
			{
				return ref new PDFMatrix(1 / m_scale, -1 / m_scale, (scrollx - m_left) / m_scale, (m_bottom - scrolly) / m_scale);
			}
		};
	}
}
