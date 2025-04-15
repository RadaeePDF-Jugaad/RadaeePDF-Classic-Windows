using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.UI;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;

namespace RDPDFReader.annotui
{
    class PopEdit : TextBox
    {
        public PopEdit() : base()
        {
            m_parent = null;
            m_is_show = false;
            Background = new SolidColorBrush(Color.FromArgb(255, 255, 255, 160));
        }
        private RelativePanel m_parent;
        private bool m_is_show;
        public void show(RelativePanel parent, Rect rect, float fsize, bool multi_line)
        {
            if (m_is_show) return;
            m_parent = parent;
            m_is_show = true;
            FontSize = fsize;
            if (multi_line) TextWrapping = Windows.UI.Xaml.TextWrapping.Wrap;
            else TextWrapping = Windows.UI.Xaml.TextWrapping.NoWrap;
            parent.Children.Add(this);
            TranslateTransform trans = new TranslateTransform();
            trans.X = rect.Left;
            trans.Y = rect.Top;
            RenderTransform = trans;
            Width = rect.Width;
            Height = rect.Height;
            UpdateLayout();
            InvalidateMeasure();
            Focus(Windows.UI.Xaml.FocusState.Keyboard);
        }
        public bool isShow()
        {
            return m_is_show;
        }
        public void dismiss()
        {
            if (!m_is_show) return;
            if(m_parent != null)
                m_parent.Children.Remove(this);
            m_is_show = false;
        }
    }
}
