using RDPDFLib.pdf;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Automation.Peers;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Navigation;

//https://go.microsoft.com/fwlink/?LinkId=234236 上介绍了“用户控件”项模板

namespace RDPDFReader
{
    public sealed partial class UIPageItem : UserControl
    {
        public int m_rotate;
        private int m_pageno;
        private PDFBmp m_bmp;
        private PDFPage m_page;
        private PDFDoc m_doc;
        private int m_status;
        private TappedEventHandler m_onDelete;

        public bool m_deleted;
        const float CELL_SZ = 200.0f;
        public UIPageItem()
        {
            this.InitializeComponent();
            m_rotate = 0;
            m_pageno = -1;
            m_bmp = null;
            m_page = null;
            m_doc = null;
            m_status = 0;
            mDelete.Tag = this;
            mRotate.Tapped += vOnTapped;
        }
        private void vOnTapped(Object sender, TappedRoutedEventArgs e)
        {
            int rotn = m_rotate & 0xffff;
            int roto = m_rotate >> 16;
            rotn += 90;
            rotn %= 360;
            m_rotate = (roto << 16) | rotn;
            RotateTransform trans = new RotateTransform();
            trans.Angle = (rotn > roto) ? rotn - roto : rotn + 360 - roto;
            Point pt;
            pt.X = 0.5;
            pt.Y = 0.5;
            mPage.RenderTransformOrigin = pt;
            mPage.RenderTransform = trans;
        }
        public void ui_init(PDFDoc doc, int pageno, TappedEventHandler onDelete)
        {
            m_doc = doc;
            m_pageno = pageno;
            m_status = 0;
            m_deleted = false;
            m_onDelete = onDelete;
            PDFPage page = m_doc.GetPage(pageno);
            m_rotate = page.Rotate;
            m_rotate = (m_rotate << 16) | m_rotate;
            float pw = m_doc.GetPageWidth(m_pageno);
            float ph = m_doc.GetPageHeight(m_pageno);

            float scale1 = CELL_SZ / pw;
            float scale2 = CELL_SZ / ph;
            if (scale1 > scale2) scale1 = scale2;
            int iw = (int)(pw * scale1);
            int ih = (int)(ph * scale1);
            m_bmp = new PDFBmp(iw, ih);
            m_bmp.Reset(0xFFFFFFFF);
            mName.Text = (m_pageno + 1).ToString();
            mDelete.Tapped += m_onDelete;
        }
        public void ui_destroy()
        {
            mRotate.Tapped -= vOnTapped;
            mDelete.Tapped -= m_onDelete;
            mDelete.Tag = null;
        }
        public void ui_clear()
        {
            m_bmp = null;
        }
        public void ui_setpos(double x, double y)
        {
            SetValue(Canvas.LeftProperty, x);
            SetValue(Canvas.TopProperty, y);
        }
        public void ui_update()
        {
            mPage.Source = m_bmp.Data;
        }
        public bool bk_render()
        {
            if (m_status < 0) return false;
            PDFPage page = m_doc.GetPage(m_pageno);
            float pw = m_doc.GetPageWidth(m_pageno);
            float ph = m_doc.GetPageHeight(m_pageno);

            float scale1 = 200.0f / pw;
            float scale2 = 200.0f / ph;
            if (scale1 > scale2) scale1 = scale2;
            PDFMatrix mat = new PDFMatrix(scale1, -scale1, 0, ph * scale1);
            m_page = page;
            page.RenderToBmp(m_bmp, mat, true, PDF_RENDER_MODE.mode_best);
            mat = null;
            if(m_status < 0)
            {
                return false;
            }
            else
            {
                m_status = 2;
                return true;
            }
        }
        public bool bk_clear()
        {
            if (m_page == null) return false;
            m_page.Close();
            m_page = null;
            return true;
        }
        public bool ui_render_start()
        {
            if (m_status == 0)
            {
                m_status = 1;
                return true;
            }
            return false;
        }
        public bool ui_render_end()
        {
            if (m_status > 0)
            {
                m_status = -1;
                return true;
            }
            return false;
        }
    }
}
