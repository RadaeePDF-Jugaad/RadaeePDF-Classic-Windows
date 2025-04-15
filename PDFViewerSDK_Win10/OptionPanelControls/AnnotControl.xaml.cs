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

namespace PDFViewerSDK_Win10.OptionPanelControls
{
    public delegate void OnButtonClickHandler(PDFViewerSDK_Win10.OptionPanelControls.AnnotControl.AnnotType type);
    public sealed partial class AnnotControl : UserControl
    {
        public event OnButtonClickHandler OnButtonClick;

        public enum AnnotType
        {
            TypeLine,
            TypeRect,
            TypeEllipse,
            TypeText,
            TypeNone,
            TypeAnnotEditBox,
        }

        public AnnotControl(bool canSave)
        {
            this.InitializeComponent();

            lineAnnotBtn.IsEnabled = canSave;
            rectAnnotBtn.IsEnabled = canSave;
            ellipseAnnotBtn.IsEnabled = canSave;
            textAnnotBtn.IsEnabled = canSave;
        }

        private void onItemTapped(Object sender, TappedRoutedEventArgs e)
        {
            Button button = sender as Button;
            String senderName = button.Name;
            if (senderName.Equals("lineAnnotBtn"))
            {
                OnButtonClick(AnnotType.TypeLine);
                //mInk = Ink_create(Global::mLineWidth, 0xff0000);
                lineAnnotBtn.IsEnabled = false;
                rectAnnotBtn.IsEnabled = false;
                ellipseAnnotBtn.IsEnabled = false;
                textAnnotBtn.IsEnabled = false;
                textEditAnnotBtn.IsEnabled = false;
            }
            else if (senderName.Equals("rectAnnotBtn"))
            {
                OnButtonClick(AnnotType.TypeRect);
                //mParent->RectStart();
                rectAnnotBtn.IsEnabled = false;
                lineAnnotBtn.IsEnabled = false;
                ellipseAnnotBtn.IsEnabled = false;
                textAnnotBtn.IsEnabled = false;
                textEditAnnotBtn.IsEnabled = false;
            }
            else if (senderName.Equals("ellipseAnnotBtn"))
            {
                OnButtonClick(AnnotType.TypeEllipse);
                //mParent->EllipseStart();
                ellipseAnnotBtn.IsEnabled = false;
                lineAnnotBtn.IsEnabled = false;
                rectAnnotBtn.IsEnabled = false;
                textAnnotBtn.IsEnabled = false;
                textEditAnnotBtn.IsEnabled = false;
            }
            else if (senderName.Equals("textAnnotBtn"))
            {
                OnButtonClick(AnnotType.TypeText);
                //mParent->TextStart();
                textAnnotBtn.IsEnabled = false;
                lineAnnotBtn.IsEnabled = false;
                rectAnnotBtn.IsEnabled = false;
                ellipseAnnotBtn.IsEnabled = false;
                textEditAnnotBtn.IsEnabled = false;
            }
            else if (senderName.Equals("textEditAnnotBtn"))
            {
                OnButtonClick(AnnotType.TypeAnnotEditBox);
                textAnnotBtn.IsEnabled = false;
                lineAnnotBtn.IsEnabled = false;
                rectAnnotBtn.IsEnabled = false;
                ellipseAnnotBtn.IsEnabled = false;
                textEditAnnotBtn.IsEnabled = false;
            }
        }

        public void ResetLayout()
        {
            lineAnnotBtn.IsEnabled = true;
            rectAnnotBtn.IsEnabled = true;
            ellipseAnnotBtn.IsEnabled = true;
            textAnnotBtn.IsEnabled = true;
            textEditAnnotBtn.IsEnabled = true;
        }
    }
}
