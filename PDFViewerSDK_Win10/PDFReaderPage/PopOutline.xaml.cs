using RDPDFLib.pdf;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

//https://go.microsoft.com/fwlink/?LinkId=234236 上介绍了“用户控件”项模板

namespace RDPDFReader.annotui
{
    public sealed partial class PopOutline : UserControl
    {
        private Button createParent()
        {
            TextBlock label = new TextBlock();
            label.Text = "<Parent";
            Button btn = new Button();
            btn.Content = label;
            label.Width = 200;
            btn.Width = 200;
            btn.Height = 30;
            btn.Click += (object sender, RoutedEventArgs e) =>
            {
                m_stack_pos--;
                list(m_stack[m_stack_pos]);
            };
            return btn;
        }
        private StackPanel createItem(PDFOutline first, PDFOutline item)
        {
            StackPanel panel = new StackPanel();
            panel.Orientation = Orientation.Horizontal;

            TextBlock label = new TextBlock();
            label.Text = item.label;
            Button btn = new Button();
            btn.Content = label;
            btn.Height = 30;
            btn.Tag = item;
            btn.Click += m_callback;

            PDFOutline child = item.GetChild();
            if (child == null)
            {
                label.Width = 200;
                btn.Width = 200;
                panel.Children.Add(btn);
            }
            else
            {
                label.Width = 160;
                btn.Width = 160;
                panel.Children.Add(btn);
                btn = new Button();
                btn.Content = ">";
                btn.Height = 30;
                btn.Width = 40;
                panel.Children.Add(btn);

                btn.Click += (object sender, RoutedEventArgs e) =>
                {
                    m_stack[m_stack_pos++] = first;
                    list(child);
                };
            }
            panel.Height = 30;
            return panel;
        }
        public PopOutline()
        {
            this.InitializeComponent();
            m_stack = new PDFOutline[256];
            m_stack_pos = 0;
        }
        private PDFDoc m_doc;
        private RoutedEventHandler m_callback;
        private PDFOutline[] m_stack;
        private int m_stack_pos;
        private void reset()
        {
            while (mContent.Children.Count > 0)
                mContent.Children.RemoveAt(0);
        }
        private void list(PDFOutline item)
        {
            reset();
            if(m_stack_pos > 0)//new a parent item
            {
                mContent.Children.Add(createParent());
            }
            PDFOutline first = item;
            while (item != null)
            {
                mContent.Children.Add(createItem(first, item));
                item = item.GetNext();
            }
            mPanel.Height = Height;
        }
        public void loadOutline(PDFDoc doc, RoutedEventHandler callback)
        {
            m_doc = doc;
            m_callback = callback;
            mPanel.Background = new SolidColorBrush(Color.FromArgb(255, 255, 255, 160));
            PDFOutline root = m_doc.GetRootOutline();
            list(root);
        }
    }
}
