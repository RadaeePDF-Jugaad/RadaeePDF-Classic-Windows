using PDFReader.util;
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
    public delegate void OnComboDialogCloseHandler(bool canceled, int selIndex);
    /// <summary>
    /// 可用于自身或导航至 Frame 内部的空白页。
    /// </summary>
    public sealed partial class FormComboControl : UserControl
    {
        public event OnComboDialogCloseHandler OnComboDialogDialogClose;

        public PDFAnnot Annot
        {
            get
            {
                return mAnnot;
            }
            set
            {
                mAnnot = value;
                if (value != null && value.ComboItemCount > 0)
                {
                    mDataSet = new ComboListDataSet();
                    mDataSet.Init(value);
                    combo_list.ItemsSource = mDataSet.Items;
                }
            }
        }

        private PDFAnnot mAnnot;
        private ComboListDataSet mDataSet;

        private int mSelIndex;
        
        public FormComboControl()
        {
            mSelIndex = -1;
            this.InitializeComponent();
        }

        public bool isShowing()
        {
            return FormComboPopup.IsOpen;
        }

        public void Show()
        {
            if (FormComboPopup.IsOpen == false)
                FormComboPopup.IsOpen = true;
        }

        public void Dismiss()
        {
            if (FormComboPopup.IsOpen == true)
                FormComboPopup.IsOpen = false;
        }

        private void OnComboListItemClicked(object sender, ItemClickEventArgs e)
        {
            ComboItem item = e.ClickedItem as ComboItem;
            mSelIndex = item.Index;
        }

        private void FormComboPopup_Loaded(Object sender, RoutedEventArgs e)
        {
            Windows.UI.Core.CoreWindow rcWindow = Windows.UI.Xaml.Window.Current.CoreWindow;
            Rect rcScreen = rcWindow.Bounds;
            FormComboPopup.HorizontalOffset = rcScreen.Width / 2 - 200;
            FormComboPopup.VerticalOffset = rcScreen.Height / 2 - 150;
        }

        private void ButtonClick(object sender, RoutedEventArgs e)
        {
            Button button = sender as Button;
            string name = button.Name;
            if (name.Equals("button_cancel"))
            {
                OnComboDialogDialogClose(true, -1);
            }
            else if (name.Equals("button_ok"))
            {
                OnComboDialogDialogClose(false, mSelIndex);
            }
        }
    }
}
