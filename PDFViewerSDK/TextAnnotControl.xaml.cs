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

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace PDFViewerSDK_Win10
{
    public delegate void OnCloseDialogHandler(String subject, String content, bool cancel, bool edit);
    public sealed partial class TextAnnotControl : UserControl
    {
        private bool mIsEdit;
        private PDFAnnot mAnnot;
        public event OnCloseDialogHandler OnCloseDialog;

        public TextAnnotControl()
        {
            this.InitializeComponent();
            mIsEdit = false;
        }

        public TextAnnotControl(String subject, String content)
        {
            InitializeComponent();
            mSubjectText.Text = subject;
            mContentText.Text = content;
            mIsEdit = true;
        }

        private void onButtonClick(Object sender, RoutedEventArgs e)
        {
            Button button = (Button)sender;
            if (button.Name.Equals("btnOK"))
            {
                int subLength = mSubjectText.Text.Length;
                int contLength = mContentText.Text.Length;
                OnCloseDialog(mSubjectText.Text, mContentText.Text, (false || !(subLength > 0 || contLength > 0)), mIsEdit);
            }
            else
                OnCloseDialog(mSubjectText.Text, mContentText.Text, true, mIsEdit);
            dismiss();
        }

        public void SetContent(String subject, String content)
        {
            if (subject != "" || content != "")
            {
                mIsEdit = true;
            }
            mSubjectText.Text = subject;
            mContentText.Text = content;
        }

        public void SetAnnot(PDFAnnot annot)
        {
            if (annot == null)
            {
                mIsEdit = false;
                mSubjectText.Text = string.Empty;
                mContentText.Text = string.Empty;
            }
            else
            {
                mAnnot = annot;
                mIsEdit = true;
                mSubjectText.Text = mAnnot.PopupSubject;
                mContentText.Text = mAnnot.PopupText;
            }
        }

        public void show()
        {
            if (!TextAnnotPopup.IsOpen)
            {
                TextAnnotPopup.IsOpen = true;
            }
        }

        public void dismiss()
        {
            if (TextAnnotPopup.IsOpen)
            {
                TextAnnotPopup.IsOpen = false;
            }
        }

        public bool isShown()
        {
            return TextAnnotPopup.IsOpen;
        }

        void TextAnnotPopup_Loaded(Object sender, RoutedEventArgs e)
        {
            TextAnnotPopup.HorizontalOffset = 50;
            TextAnnotPopup.VerticalOffset = 50;
        }
    }
}
