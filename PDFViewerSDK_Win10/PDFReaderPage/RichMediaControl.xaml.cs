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

namespace RDPDFReader.annotui
{
    public delegate void OnRichmediaControlButtonClickHandler(int buttonCode);

    public sealed partial class RichMediaControl : UserControl
    {
        public event OnRichmediaControlButtonClickHandler OnButtonClick;
        public RichMediaControl()
        {
            this.InitializeComponent();
        }
        public MediaElement getPlayer()
        {
            return mPlayer;
        }
        private void onItemTapped(Object sender, TappedRoutedEventArgs e)
        {
            Button button = sender as Button;
            int buttonCode = -1;
            if (button.Equals(mPlayBtn))
            {
                buttonCode = 0;
            }
            else if (button.Equals(mPauseBtn))
            {
                buttonCode = 1;
            }
            else if (button.Equals(mStopBtn))
            {
                buttonCode = 2;
            }
            OnButtonClick(buttonCode);
        }
    }
}
