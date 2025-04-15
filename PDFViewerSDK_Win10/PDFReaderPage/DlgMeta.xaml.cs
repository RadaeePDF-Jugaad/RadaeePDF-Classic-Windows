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

// https://go.microsoft.com/fwlink/?LinkId=234238 上介绍了“内容对话框”项模板

namespace RDPDFReader.annotui
{
    public sealed partial class DlgMeta : ContentDialog
    {
        public DlgMeta()
        {
            this.InitializeComponent();
        }
        private PDFDoc m_doc;
        public void loadMeta(PDFDoc doc)
        {
            m_doc = doc;
            mTitle.Text = m_doc.GetMeta("Title");
            mAuthor.Text = m_doc.GetMeta("Author");
            mSubject.Text = m_doc.GetMeta("Subject");
            mKeywords.Text = m_doc.GetMeta("Keywords");
            mCreator.Text = m_doc.GetMeta("Creator");
            mProducer.Text = m_doc.GetMeta("Producer");
            mPDFA.Text = m_doc.GetMeta("pdf/a");
            mCreate.Text = m_doc.GetMeta("CreationDate");
            mModify.Text = m_doc.GetMeta("ModDate");
        }
        private void ContentDialog_PrimaryButtonClick(ContentDialog sender, ContentDialogButtonClickEventArgs args)
        {
            m_doc.SetMeta("Title", mTitle.Text);
            m_doc.SetMeta("Author", mAuthor.Text);
            m_doc.SetMeta("Subject", mSubject.Text);
            m_doc.SetMeta("Keywords", mKeywords.Text);
            m_doc.SetMeta("Creator", mCreator.Text);
            m_doc.SetMeta("Producer", mProducer.Text);
        }

        private void ContentDialog_SecondaryButtonClick(ContentDialog sender, ContentDialogButtonClickEventArgs args)
        {
        }
    }
}
