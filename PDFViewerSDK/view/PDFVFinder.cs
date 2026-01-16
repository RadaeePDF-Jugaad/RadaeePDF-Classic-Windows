using System;
using RDPDFLib.pdf;
using System.Threading;
using System.Windows;
using Windows.UI;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Shapes;

namespace PDFViewerSDK_Win10
{
    namespace view
    {
        public class PDFVFinder
        {
            private String m_str = null;
            private Boolean m_case = false;
            private Boolean m_whole = false;
            private int m_page_no = -1;
            private int m_page_find_index = -1;
            private int m_page_find_cnt = 0;
            private PDFPage m_page = null;
            private PDFDoc m_doc = null;
	
            private PDFFinder m_finder = null;
	
            private int m_dir = 0;
            private Boolean is_cancel = true;
            private ManualResetEventSlim m_eve = new ManualResetEventSlim();
            public PDFVFinder()
            {
	            //m_paint.setARGB(0x40, 0, 0, 255);
	            //m_paint.setStyle(Style.FILL);
            }
            private void eve_reset()
            {
                m_eve.Reset();
            }
            private void eve_wait()
            {
                m_eve.Wait();
            }
            private void eve_notify()
            {
                m_eve.Set();
            }
            public void find_start(PDFDoc doc, int page_start, String str, Boolean match_case, Boolean whole)
            {
	            m_str = str;
	            m_case = match_case;
	            m_whole = whole;
	            m_doc = doc;
	            m_page_no = page_start;
	            if( m_page != null )
	            {
		            if( m_finder != null )
		            {
			            m_finder = null;
		            }
                    m_page.Close();
                    m_page = null;
                }
	            m_page_find_index = -1;
	            m_page_find_cnt = 0;
            }
            public int find_prepare(int dir)
            {
	            if( m_str == null ) return 0;
	            if( !is_cancel ) eve_wait();
	            m_dir = dir;
	            eve_reset();
	            if( m_page == null )
	            {
		            is_cancel = false;
		            return -1;
	            }
	            is_cancel = true;
	            if( dir < 0 )
	            {
		            if( m_page_find_index >= 0) m_page_find_index--;
		            if( m_page_find_index < 0 )
		            {
			            if( m_page_no <= 0 )
			            {
				            return 0;
			            }
			            else
			            {
				            is_cancel = false;
				            return -1;
			            }
		            }
		            else
			            return 1;
	            }
	            else
	            {
		            if( m_page_find_index < m_page_find_cnt ) m_page_find_index++;
		            if( m_page_find_index >= m_page_find_cnt )
		            {
			            if( m_page_no >= m_doc.PageCount - 1 )
			            {
				            return 0;
			            }
			            else
			            {
				            is_cancel = false;
				            return -1;
			            }
		            }
		            else
			            return 1;
	            }
            }
            public int find()
            {
	            int ret = 0;
                int pcnt = m_doc.PageCount;
	            if( m_dir < 0 )
	            {
		            while( (m_page == null || m_page_find_index < 0) && m_page_no >= 0 && !is_cancel )
		            {
			            if( m_page == null )
			            {
				            if( m_page_no >= pcnt ) m_page_no = pcnt - 1;
				            m_page = m_doc.GetPage(m_page_no);
				            m_page.ObjsStart();
				            m_finder = m_page.GetFinder(m_str, m_case, m_whole);
				            if( m_finder == null ) m_page_find_cnt = 0;
				            else m_page_find_cnt = m_finder.GetCount();
				            m_page_find_index = m_page_find_cnt - 1;
			            }
			            if( m_page_find_index < 0 )
			            {
				            if( m_finder != null )
				            {
					            m_finder = null;
				            }
                            if (m_page != null)
                            {
                                m_page.Close();
                                m_page = null;
                            }
                            m_page_find_cnt = 0;
				            m_page_no--;
			            }
		            }
		            if( is_cancel || m_page_no < 0 )
		            {
			            if( m_finder != null )
			            {
				            m_finder = null;
			            }
                        if (m_page != null)
                        {
                            m_page.Close();
                            m_page = null;
                        }
                        ret = 0;//find error, notify UI process
		            }
		            else
			            ret = 1;//find finished, notify UI process
	            }
	            else
	            {
		            while( (m_page == null || m_page_find_index >= m_page_find_cnt) && m_page_no < pcnt && !is_cancel )
		            {
			            if( m_page == null )
			            {
				            if( m_page_no < 0 ) m_page_no = 0;
				            m_page = m_doc.GetPage(m_page_no);
				            m_page.ObjsStart();
                            m_finder = m_page.GetFinder(m_str, m_case, m_whole);
				            if( m_finder == null ) m_page_find_cnt = 0;
				            else m_page_find_cnt = m_finder.GetCount();
				            m_page_find_index = 0;
			            }
			            if( m_page_find_index >= m_page_find_cnt )
			            {
				            if( m_finder != null )
				            {
					            m_finder = null;
				            }
                            if (m_page != null)
                            {
                                m_page.Close();
                                m_page = null;
                            }
                            m_page_find_cnt = 0;
				            m_page_no++;
			            }
		            }
		            if( is_cancel || m_page_no >= pcnt )
		            {
			            if( m_finder != null )
			            {
				            m_finder = null;
			            }
                        if (m_page != null)
                        {
                            m_page.Close();
                            m_page = null;
                        }
                        ret = 0;////find error, notify UI process
		            }
		            else
			            ret = 1;//find finished, notify UI process
	            }
	            eve_notify();
	            return ret;
            }
            public PDFRect find_get_pos()//get current found's bound.
            {
                PDFRect rect;
                rect.left = 0;
                rect.top = 0;
                rect.right = 0;
                rect.bottom = 0;
                if (m_finder != null)
	            {
		            int ichar = m_finder.GetFirstChar(m_page_find_index);
                    if (ichar < 0)
                    {
                        return rect;
                    }
		            return m_page.ObjsGetCharRect(ichar);
	            }
	            else
		            return rect;
            }
            public void find_draw(PDFVCanvas canvas, PDFVPage page, float scrollx, float scrolly, float offsetx, float offsety)//draw current found
            {
	            if( !is_cancel )
	            {
		            eve_wait();
		            is_cancel = true;
	            }
	            if( m_str == null ) return;
                Color clr = new Color();
                clr.R = 0;
                clr.A = 0x40;
                clr.B = 255;
                clr.G = 0;
	            if( m_finder != null && m_page_find_index >= 0 && m_page_find_index < m_page_find_cnt )
	            {
		            int ichar = m_finder.GetFirstChar(m_page_find_index);
		            int ichar_end = ichar + m_str.Length;
                    PDFRect rect;
                    PDFRect rect_word;
                    PDFRect rect_draw;
                    rect = m_page.ObjsGetCharRect(ichar);
                    rect_word = rect;
                    ichar++;

                    while ( ichar < ichar_end )
		            {
                        rect = m_page.ObjsGetCharRect(ichar);
                        float gap = (rect.bottom - rect.top) / 2;
                        if (rect_word.top == rect.top && rect_word.bottom == rect.bottom &&
				            rect_word.right + gap > rect.left && rect_word.left - gap < rect.right )
			            {
                            if (rect_word.left > rect.left) rect_word.left = rect.left;
                            if (rect_word.right < rect.right) rect_word.right = rect.right;
			            }
			            else
			            {
                            //rect_draw.left = page.ToDIBX(rect_word.left) + page.GetX() - scrollx;
                            //rect_draw.top = page.ToDIBY(rect_word.bottom) + page.GetY() - scrolly;
                            //rect_draw.right = page.ToDIBX(rect_word.right) + page.GetX() - scrollx;
                            //rect_draw.bottom = page.ToDIBY(rect_word.top) + page.GetY() - scrolly;
                            rect_draw.left = page.ToDIBX(rect_word.left) + page.GetX() - scrollx + offsetx;
                            rect_draw.top = page.ToDIBY(rect_word.bottom) + page.GetY() - scrolly + offsety;
                            rect_draw.right = page.ToDIBX(rect_word.right) + page.GetX() - scrollx + offsetx;
                            rect_draw.bottom = page.ToDIBY(rect_word.top) + page.GetY() - scrolly + offsety;
                            canvas.fill_rect(rect_draw, clr);
				            rect_word = rect;
			            }
			            ichar++;
		            }
                    //rect_draw.left = page.ToDIBX(rect_word.left) + page.GetX() - scrollx;
                    //rect_draw.top = page.ToDIBY(rect_word.bottom) + page.GetY() - scrolly;
                    //rect_draw.right = page.ToDIBX(rect_word.right) + page.GetX() - scrollx;
                    //rect_draw.bottom = page.ToDIBY(rect_word.top) + page.GetY() - scrolly;
                    rect_draw.left = page.ToDIBX(rect_word.left) + page.GetX() - scrollx + offsetx;
                    rect_draw.top = page.ToDIBY(rect_word.bottom) + page.GetY() - scrolly + offsety;
                    rect_draw.right = page.ToDIBX(rect_word.right) + page.GetX() - scrollx + offsetx;
                    rect_draw.bottom = page.ToDIBY(rect_word.top) + page.GetY() - scrolly + offsety;
                    canvas.fill_rect(rect_draw, clr);
                }
            }
            public int find_get_page()//get current found's page NO
            {
	            return m_page_no;
            }
            public void find_end()
            {
	            if( !is_cancel )
	            {
		            is_cancel = true;
		            eve_wait();
	            }
	            m_str = null;
	            if( m_page != null )
	            {
		            if( m_finder != null )
		            {
			            m_finder = null;
		            }
                    m_page.Close();
                    m_page = null;
                }
            }
        }
    }
}