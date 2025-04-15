using com.radaee.reader;
using RDPDFLib.pdf;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

//https://go.microsoft.com/fwlink/?LinkId=234236 上介绍了“用户控件”项模板

namespace RDPDFReader
{
    public delegate void OnPageConfirmHandler(int[] rotates, bool[] deletes);
    public sealed partial class PDFPagesView : UserControl
    {
        private PDFPageList m_view;
        private OnPageConfirmHandler m_callback;
        public PDFPagesView()
        {
            this.InitializeComponent();
            m_view = new PDFPageList();
        }
        public void PDFOpen(PDFDoc doc, OnPageConfirmHandler callback)
        {
            m_view.PDFOpen(mView, doc);
            m_callback = callback;
            Visibility = Visibility.Visible;
        }
        private void mBack_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            m_view.PDFClose();
            Visibility = Visibility.Collapsed;
        }
        private void mConfirm_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            m_callback(m_view.PDFGetRotate(), m_view.PDFGetRemove());
            m_view.PDFClose();
            Visibility = Visibility.Collapsed;
        }
    }
}
