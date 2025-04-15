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
    class MenuAnnot : StackPanel
    {
        private Button m_btn_perform;
        private Button m_btn_edit;
        private Button m_btn_remove;
        private Button m_btn_property;
        private bool m_is_show;
        public MenuAnnot() : base()
        {
            m_btn_perform = new Button();
            m_btn_edit = new Button();
            m_btn_remove = new Button();
            m_btn_property = new Button();
            Orientation = Orientation.Horizontal;
            Children.Add(m_btn_perform);
            Children.Add(m_btn_edit);
            Children.Add(m_btn_remove);
            Children.Add(m_btn_property);
            m_btn_perform.Content = "Perform";
            m_btn_edit.Content = "Edit";
            m_btn_remove.Content = "Remove";
            m_btn_property.Content = "Property";
            m_btn_perform.Height = 40;
            m_btn_edit.Height = 40;
            m_btn_remove.Height = 40;
            m_btn_property.Height = 40;
            Background = new SolidColorBrush(Color.FromArgb(255, 255, 255, 224));
            m_is_show = false;
        }
        public void init(RoutedEventHandler onPerform, RoutedEventHandler onEdit, RoutedEventHandler onRemove, RoutedEventHandler onProperty)
        {
            m_btn_perform.Click += onPerform;
            m_btn_edit.Click += onEdit;
            m_btn_remove.Click += onRemove;
            m_btn_property.Click += onProperty;
        }
        public void uninit(RoutedEventHandler onPerform, RoutedEventHandler onEdit, RoutedEventHandler onRemove, RoutedEventHandler onProperty)
        {
            m_btn_perform.Click -= onPerform;
            m_btn_edit.Click -= onEdit;
            m_btn_remove.Click -= onRemove;
            m_btn_property.Click -= onProperty;
            Children.Clear();
        }
        private RelativePanel m_parent;
        public void show(RelativePanel parent, int atype, Point pt)
        {
            m_is_show = (atype != 20 && atype != 3);
            bool has_perform = (atype == 2 || atype == 17 || atype == 18 || atype == 19 || atype == 25 || atype == 26);
            bool has_edit = (atype == 1 || atype == 4 || atype == 5 || atype == 6 ||
                    atype == 7 || atype == 8 || atype == 9 || atype == 10 ||
                    atype == 11 || atype == 12 || atype == 13 || atype == 15);
            bool has_remove = (atype != 0);
            bool has_property = (atype != 0 && atype != 2);

            if (has_perform) m_btn_perform.Visibility = Windows.UI.Xaml.Visibility.Visible;
            else m_btn_perform.Visibility = Windows.UI.Xaml.Visibility.Collapsed;

            if (has_edit) m_btn_edit.Visibility = Windows.UI.Xaml.Visibility.Visible;
            else m_btn_edit.Visibility = Windows.UI.Xaml.Visibility.Collapsed;

            if (has_remove) m_btn_remove.Visibility = Windows.UI.Xaml.Visibility.Visible;
            else m_btn_remove.Visibility = Windows.UI.Xaml.Visibility.Collapsed;

            if (has_property) m_btn_property.Visibility = Windows.UI.Xaml.Visibility.Visible;
            else m_btn_property.Visibility = Windows.UI.Xaml.Visibility.Collapsed;

            if (!m_is_show) return;
            m_parent = parent;
            parent.Children.Add(this);
            TranslateTransform trans = new TranslateTransform();
            trans.X = pt.X;
            trans.Y = pt.Y;
            RenderTransform = trans;
            Height = 40;
            UpdateLayout();
            InvalidateMeasure();
            double windowWidth = Window.Current.Bounds.Width;
            if (trans.X + ActualWidth > windowWidth)
            {
                trans.X = windowWidth - ActualWidth;
                UpdateLayout();
            }
        }
        public void dismiss()
        {
            if (!m_is_show) return;
            if (m_parent != null)
                m_parent.Children.Remove(this);
        }
    }
}
