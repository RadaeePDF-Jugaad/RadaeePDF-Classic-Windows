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
    public delegate void OnDialogCloseHandler(string password);

    public sealed partial class PasswordControl : UserControl
    {
        public event OnDialogCloseHandler OnDialogClose;
        public PasswordControl()
        {
            this.InitializeComponent();
        }

        public void showHint()
        {
            password_box.Password = "";
            if (password_hint.Visibility == Windows.UI.Xaml.Visibility.Collapsed)
                password_hint.Visibility = Windows.UI.Xaml.Visibility.Visible;
        }

        public bool isShowing()
        {
            return PasswordPopup.IsOpen;
        }

        public void Show()
        {
            if (PasswordPopup.IsOpen == false)
                PasswordPopup.IsOpen = true;
        }

        public void Dismiss()
        {
            if (PasswordPopup.IsOpen == true)
                PasswordPopup.IsOpen = false;
        }

        private void PasswordPopup_Loaded(Object sender, RoutedEventArgs e)
        {
            Windows.UI.Core.CoreWindow rcWindow = Windows.UI.Xaml.Window.Current.CoreWindow;
            Rect rcScreen = rcWindow.Bounds;
            PasswordPopup.HorizontalOffset = rcScreen.Width / 2 - 200;
            PasswordPopup.VerticalOffset = rcScreen.Height / 2 - 150;
        }

        private void ButtonClick(object sender, RoutedEventArgs e)
        {
            Button button = sender as Button;
            string name = button.Name;
            if (name.Equals("button_cancel"))
            {
                OnDialogClose("");
            }
            else if (name.Equals("button_ok"))
            {
                OnDialogClose(password_box.Password);
            }
        }
    }
}
