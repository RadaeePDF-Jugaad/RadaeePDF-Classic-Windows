using com.radaee.reader;
using RDPDFLib.pdf;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Media.Imaging;

namespace RDPDFReader.annotui
{
    class UILStyleView : Button
    {
        public UILStyleView() : base()
        {
            m_bmp = new WriteableBitmap(60, 30);
            m_img = new Image();
            m_img.Width = 60;
            m_img.Height = 30;
            Content = m_img;
            Padding = new Windows.UI.Xaml.Thickness(0, 0, 0, 0);
            BorderThickness = new Windows.UI.Xaml.Thickness(0, 0, 0, 0);
            setDash(null);
        }
        private WriteableBitmap m_bmp;
        private float[] m_dash;
        private Image m_img;
        public float[] getDash()
        {
            return m_dash;
        }
        public void setDash(float[] dash)
        {
            m_dash = dash;
            RDPDFLib.pdf.PDFGlobal.drawDashLine(m_dash, m_bmp);
            m_img.Source = m_bmp;
            m_bmp.Invalidate();
        }
    }
    class UILStyleButton : Button
    {
        public UILStyleButton() : base()
        {
            m_bmp = new WriteableBitmap(60, 30);
            m_img = new Image();
            m_img.Width = 60;
            m_img.Height = 30;
            Content = m_img;
            Padding = new Windows.UI.Xaml.Thickness(0, 0, 0, 0);
            BorderThickness = new Windows.UI.Xaml.Thickness(0, 0, 0, 0);
            setDash(null);

            m_panel = new StackPanel();
            m_panel.Width = 60;
            m_panel.Height = 6 * 30;
            m_panel.Orientation = Orientation.Vertical;
            
            RoutedEventHandler click = (object sender, RoutedEventArgs e) =>
            {
                UILStyleView vv = (UILStyleView)sender;
                setDash(vv.getDash());
                if(m_popup != null)
                {
                    m_popup.dismiss();
                    m_popup = null;
                }
            };

            UILStyleView vw;
            vw = new UILStyleView();
            vw.Width = 60;
            vw.Height = 30;
            vw.Click += click;
            m_panel.Children.Add(vw);

            vw = new UILStyleView();
            vw.Width = 60;
            vw.Height = 30;
            vw.Click += click;
            vw.setDash(new float[] { 1, 1 });
            m_panel.Children.Add(vw);

            vw = new UILStyleView();
            vw.Width = 60;
            vw.Height = 30;
            vw.Click += click;
            vw.setDash(new float[] { 2, 2 });
            m_panel.Children.Add(vw);

            vw = new UILStyleView();
            vw.Width = 60;
            vw.Height = 30;
            vw.Click += click;
            vw.setDash(new float[] { 4, 4 });
            m_panel.Children.Add(vw);

            vw = new UILStyleView();
            vw.Width = 60;
            vw.Height = 30;
            vw.Click += click;
            vw.setDash(new float[] { 4, 2, 2, 2 });
            m_panel.Children.Add(vw);

            vw = new UILStyleView();
            vw.Width = 60;
            vw.Height = 30;
            vw.Click += click;
            vw.setDash(new float[] { 12, 2, 4, 2 });
            m_panel.Children.Add(vw);
            
        }
        private WriteableBitmap m_bmp;
        private float[] m_dash;
        private Image m_img;
        private StackPanel m_panel;
        public float[] getDash()
        {
            return m_dash;
        }
        public void setDash(float[] dash)
        {
            m_dash = dash;
            RDPDFLib.pdf.PDFGlobal.drawDashLine(m_dash, m_bmp);
            m_img.Source = m_bmp;
            m_bmp.Invalidate();
        }
        private RelativePanel m_root;
        private PDFPopup m_popup;
        public void init(RelativePanel root)
        {
            if (m_popup != null) m_popup.dismiss();
            m_root = root;
            Click += (object sender, RoutedEventArgs e) =>
            {
                GeneralTransform trans = TransformToVisual(m_root);
                Point pt;
                //pt.X = ActualWidth;
                pt.X = 0;
                pt.Y = 0;
                pt = trans.TransformPoint(pt);
                Rect rect = new Rect(pt.X, pt.Y, 60, 6 * 30);
                m_popup = new PDFPopup(m_root, m_panel, rect);
            };
        }
    }
}