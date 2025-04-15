using System;
using RDPDFLib.pdf;
using Windows.UI;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Shapes;
namespace PDFViewerSDK_Win10
{
    namespace view
    {
        public class PDFVSel
        {
            protected PDFPage m_page;
            public int m_index1;
            public int m_index2;
            protected Boolean m_ok = false;
            protected Boolean m_swiped = false;
            public PDFVSel(PDFPage page)
            {
                m_page = page;
                m_index1 = -1;
                m_index2 = -1;
            }
            public void Clear()
            {
                if (m_page != null)
                {
                    m_page.Close();
                    m_page = null;
                }
            }
            public PDFRect GetRect1(float scale, float page_height, float orgx, float orgy)
	        {
                PDFRect rect;
                if (m_index1 < 0 || m_index2 < 0 || !m_ok)
                {
                    rect.left = 0;
                    rect.top = 0;
                    rect.right = 0;
                    rect.bottom = 0;
                    return rect;
                }
		        if( m_swiped )
			        rect = m_page.ObjsGetCharRect(m_index2);
		        else
			        rect = m_page.ObjsGetCharRect(m_index1);
		        PDFRect rect_draw;
		        rect_draw.left = (rect.left * scale) + orgx;
		        rect_draw.top = ((page_height - rect.bottom) * scale) + orgy;
		        rect_draw.right = (rect.right * scale) + orgx;
		        rect_draw.bottom = ((page_height - rect.top) * scale) + orgy;
		        return rect_draw;
	        }
            public PDFRect GetRect2(float scale, float page_height, float orgx, float orgy)
	        {
                PDFRect rect;
                if (m_index1 < 0 || m_index2 < 0 || !m_ok)
                {
                    rect.left = 0;
                    rect.top = 0;
                    rect.right = 0;
                    rect.bottom = 0;
                    return rect;
                }
		        if( m_swiped )
			        rect = m_page.ObjsGetCharRect(m_index1);
		        else
			        rect = m_page.ObjsGetCharRect(m_index2);
                PDFRect rect_draw;
                rect_draw.left = (rect.left * scale) + orgx;
                rect_draw.top = ((page_height - rect.bottom) * scale) + orgy;
                rect_draw.right = (rect.right * scale) + orgx;
                rect_draw.bottom = ((page_height - rect.top) * scale) + orgy;
                return rect_draw;
            }
            public void SetSel(float x1, float y1, float x2, float y2)
	        {
		        if( !m_ok )
		        {
			        m_page.ObjsStart();
			        m_ok = true;
		        }
		        m_index1 = m_page.ObjsGetCharIndex(x1, y1);
		        m_index2 = m_page.ObjsGetCharIndex(x2, y2);
		        if( m_index1 > m_index2 )
		        {
			        int tmp = m_index1;
			        m_index1 = m_index2;
			        m_index2 = tmp;
			        m_swiped = true;
		        }
		        else
			        m_swiped = false;
		        m_index1 = m_page.ObjsAlignWord(m_index1, -1);
		        m_index2 = m_page.ObjsAlignWord(m_index2, 1);
	        }
            public Boolean SetSelMarkup(uint color, int type)
            {
                if (m_index1 < 0 || m_index2 < 0 || !m_ok) return false;
                return m_page.AddAnnotMarkup(m_index1, m_index2, color, type);
            }
            public String GetSelString()
            {
                if (m_index1 < 0 || m_index2 < 0 || !m_ok) return null;
                return m_page.ObjsGetString(m_index1, m_index2 + 1);
            }
            public void DrawSel(PDFVCanvas canvas, float scale, float page_height, float orgx, float orgy)
	        {
		        if( m_index1 < 0 || m_index2 < 0 || !m_ok ) return;
		        PDFRect rect;
		        PDFRect rect_word;
		        PDFRect rect_draw;
                Color clr = new Color();
                clr.A = 0x40;
                clr.B = 0xFF;
                clr.G = 0;
                clr.R = 0;
                rect = m_page.ObjsGetCharRect(m_index1);
                rect_word = rect;
		        int tmp = m_index1 + 1;
		        while( tmp <= m_index2 )
		        {
                    rect = m_page.ObjsGetCharRect(tmp);
			        float gap = (rect.bottom - rect.top)/2;
			        if( rect_word.top == rect.top && rect_word.bottom == rect.bottom &&
				        rect_word.right + gap > rect.left && rect_word.left - gap < rect.right )
			        {
				        if( rect_word.left > rect.left ) rect_word.left = rect.left;
				        if( rect_word.right < rect.right ) rect_word.right = rect.right;
			        }
			        else
			        {
                        rect_draw.left = rect_word.left * scale + orgx;
                        rect_draw.top = (page_height - rect_word.bottom) * scale + orgy;
                        rect_draw.right = rect_word.right * scale + orgx;
                        rect_draw.bottom = (page_height - rect_word.top) * scale + orgy;
                        canvas.fill_rect(rect_draw, clr);
				        rect_word = rect;
			        }
			        tmp++;
		        }
                rect_draw.left = rect_word.left * scale + orgx;
                rect_draw.top = (page_height - rect_word.bottom) * scale + orgy;
                rect_draw.right = rect_word.right * scale + orgx;
                rect_draw.bottom = (page_height - rect_word.top) * scale + orgy;
                canvas.fill_rect(rect_draw, clr);
            }
            public void DrawSel(PDFVCanvas canvas, PDFVPage page)
	        {
		        if( m_index1 < 0 || m_index2 < 0 || !m_ok ) return;
                PDFRect rect;
                PDFRect rect_word;
                PDFRect rect_draw;
                Color clr = new Color();
                clr.A = 0x40;
                clr.B = 0xFF;
                clr.G = 0;
                clr.R = 0;
                rect = m_page.ObjsGetCharRect(m_index1);
		        rect_word = rect;
		        int tmp = m_index1 + 1;
		        while( tmp <= m_index2 )
		        {
			        rect = m_page.ObjsGetCharRect(tmp);
                    float gap = (rect.bottom - rect.top) / 2;
                    if (rect_word.top == rect.top && rect_word.bottom == rect.bottom &&
                        rect_word.right + gap > rect.left && rect_word.left - gap < rect.right)
                    {
                        if (rect_word.left > rect.left) rect_word.left = rect.left;
                        if (rect_word.right < rect.right) rect_word.right = rect.right;
                    }
			        else
			        {
				        rect_draw.left = page.ToDIBX(rect_word.left) + page.GetX();
				        rect_draw.top = page.ToDIBY(rect_word.top) + page.GetY();
				        rect_draw.right = page.ToDIBX(rect_word.right) + page.GetX();
				        rect_draw.bottom = page.ToDIBY(rect_word.bottom) + page.GetY();
                        canvas.fill_rect(rect_draw, clr);
                        rect_word = rect;
			        }
			        tmp++;
		        }
		        rect_draw.left = page.ToDIBX(rect_word.left) + page.GetX();
		        rect_draw.top = page.ToDIBY(rect_word.top) + page.GetY();
		        rect_draw.right = page.ToDIBX(rect_word.right) + page.GetX();
		        rect_draw.bottom = page.ToDIBY(rect_word.bottom) + page.GetY();
                canvas.fill_rect(rect_draw, clr);
            }
        }
    }
}