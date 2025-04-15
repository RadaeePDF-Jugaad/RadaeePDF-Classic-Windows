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

// https://go.microsoft.com/fwlink/?LinkId=234238 上介绍了“空白页”项模板

namespace PDFViewerSDK_Win10.AnnotControls
{
    public delegate void OnEditDialogCloseHandler(bool canceled, string inputText);

    /// <summary>
    /// 可用于自身或导航至 Frame 内部的空白页。
    /// </summary>
    public sealed partial class FormEditControl : UserControl
    {

        public event OnEditDialogCloseHandler OnEditDialogDialogClose;

        public PDFAnnot Annot {
            get {
                return mAnnot;
            }
            set {
                mAnnot = value;
                if (value != null && text_box != null)
                {
                    mEditType = value.EditType;
                    switch (mEditType)
                    {
                        case 1:
                            //Single line input
                            text_box.AcceptsReturn = false;
                            break;
                        case 2:
                            //Password input
                            text_box.Visibility = Visibility.Collapsed;
                            password_box.Visibility = Visibility.Visible;
                            break;
                        case 3:
                            //Multi-line input
                            text_box.AcceptsReturn = true;
                            break;
                    }
                    text_box.Text = value.EditText;
                    
                }
            }
        }

        private PDFAnnot mAnnot;
        private int mEditType;

        public FormEditControl()
        {
            this.InitializeComponent();
        }

        public bool isShowing()
        {
            return FormEditPopup.IsOpen;
        }

        public void Show()
        {
            if (FormEditPopup.IsOpen == false)
                FormEditPopup.IsOpen = true;
        }

        public void Dismiss()
        {
            if (FormEditPopup.IsOpen == true)
                FormEditPopup.IsOpen = false;
        }

        private void FormEditPopup_Loaded(Object sender, RoutedEventArgs e)
        {
            Windows.UI.Core.CoreWindow rcWindow = Windows.UI.Xaml.Window.Current.CoreWindow;
            Rect rcScreen = rcWindow.Bounds;
            FormEditPopup.HorizontalOffset = rcScreen.Width / 2 - 200;
            FormEditPopup.VerticalOffset = rcScreen.Height / 2 - 150;
        }

        private void ButtonClick(object sender, RoutedEventArgs e)
        {
            Button button = sender as Button;
            string name = button.Name;
            if (name.Equals("button_cancel"))
            {
                OnEditDialogDialogClose(true, "");
            }
            else if (name.Equals("button_ok"))
            {
                String text;
                if (mEditType == 2)
                    text = password_box.Password;
                else
                    text = text_box.Text;
                OnEditDialogDialogClose(false, text);
            }
        }
    }
}
