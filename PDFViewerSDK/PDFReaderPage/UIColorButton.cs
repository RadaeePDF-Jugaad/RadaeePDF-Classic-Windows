using com.radaee.reader;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;

namespace RDPDFReader.annotui
{
    class UIColorButton : Button
    {
        public UIColorButton() : base()
        {
        }
        private RelativePanel m_root;
        private uint m_color;
        private bool m_has_enable;
        public uint getColor()
        {
            return m_color;
        }
        public void setColor(uint color)
        {
            m_color = color;
            Background = new SolidColorBrush(Color.FromArgb(255, (byte)(color >> 16), (byte)(color >> 8), (byte)color));
        }
        public void setHasEnable(bool has)
        {
            m_has_enable = has;
        }
        public void init(RelativePanel root)
        {
            m_root = root;
            Click += (object sender, RoutedEventArgs e) =>
            {
                GeneralTransform trans = TransformToVisual(m_root);
                Point pt;
                pt.X = 0;
                pt.Y = 0;
                pt = trans.TransformPoint(pt);
                UIColorView view = new UIColorView();
                
                Rect rect = new Rect(pt.X, pt.Y, 210, m_has_enable?230:180);
                PDFPopup pop = new PDFPopup(m_root, view, rect);
                view.init(m_color, m_has_enable, (object sender1, RoutedEventArgs e1) => {
                    setColor(view.getColor());
                    pop.dismiss();
                },
                (object sender1, RoutedEventArgs e1) => { pop.dismiss(); });
            };
        }
    }
}
