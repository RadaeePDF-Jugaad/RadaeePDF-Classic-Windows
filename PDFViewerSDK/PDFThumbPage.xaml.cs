using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Navigation;
using RDPDFLib.pdf;
using PDFViewerSDK_Win10.view;
using System;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace PDFViewerSDK_Win10
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class PDFThumbPage : Page, PDFView.PDFViewListener
    {
        private PDFDoc m_doc;
        private PDFView m_view;
        public PDFThumbPage()
        {
            this.InitializeComponent();
        }

        private void backButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            goback();
            //if (m_view != null) m_view.vClose();
            //Frame.GoBack();
        }

        private void OnBtnGoBack(object sender, RoutedEventArgs e)
        {
            goback();
            //if (m_view != null) m_view.vClose();
            //Frame.GoBack();
        }
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            m_doc = (PDFDoc)e.Parameter;
            if (m_doc != null)
            {
                m_view = new PDFViewThumb(m_canvas, 0);
                m_view.vOpen(m_doc, 4, 0xFFCCCCCC, this);
            }
        }

        private void goback(int page = -1)
        {
            if (page != -1)
            {
                //page--;
                PDFView.PDFPos pos = m_view.vGetPos(0, 0);
                if (!Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey(MainPage.FileToken + "_page"))
                {
                    Windows.Storage.ApplicationData.Current.LocalSettings.Values.Add(MainPage.FileToken + "_page", page);
                }
                else
                {
                    Windows.Storage.ApplicationData.Current.LocalSettings.Values[MainPage.FileToken + "_page"] = page;
                }

                if (!Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey(MainPage.FileToken + "_x"))
                {
                    Windows.Storage.ApplicationData.Current.LocalSettings.Values.Add(MainPage.FileToken + "_x", pos.x);
                }
                else
                {
                    Windows.Storage.ApplicationData.Current.LocalSettings.Values[MainPage.FileToken + "_x"] = pos.x;
                }
                                        
                if (!Windows.Storage.ApplicationData.Current.LocalSettings.Values.ContainsKey(MainPage.FileToken + "_y"))
                {
                    Windows.Storage.ApplicationData.Current.LocalSettings.Values.Add(MainPage.FileToken + "_y", pos.y);
                }
                else
                {
                    Windows.Storage.ApplicationData.Current.LocalSettings.Values[MainPage.FileToken + "_y"] = pos.y;
                }
            }
            if (m_view != null) m_view.vClose();
            Frame.GoBack();
        }

        public void OnPDFPageChanged(int pageno)
        {
        }

        public bool OnPDFSingleTapped(float x, float y)
        {
            return false;
        }

        public void OnPDFLongPressed(float x, float y)
        {
        }

        public void OnPDFFound(bool found)
        {
        }

        public void OnPDFPageDisplayed(Canvas canvas, PDFVPage vpage)
        {
        }

        public void OnPDFSelecting(Canvas canvas, PDFRect rect1, PDFRect rect2)
        {
        }

        public void OnPDFSelected()
        {
        }

        public void OnPDFAnnotClicked(PDFPage page, PDFAnnot annot)
        {
        }

        public void OnPDFAnnotEnd()
        {
        }

        public void OnPDFAnnotGoto(int pageno)
        {
        }

        public void OnPDFAnnotURI(string uri)
        {
        }

        //public void OnPDFAnnotMovieAsync(PDFAnnot annot, string name)
        //{
        //}

        //public void OnPDFAnnotSound(PDFAnnot annot, string name)
        //{
        //}

        public void OnPDFAnnotPopup(PDFAnnot annot, string subj, string text)
        {
        }

        public void OnPDFAnnotRemoteDest(string dest)
        {
        }

        public void OnPDFAnnotFileLink(string filelink)
        {
        }

        public void OnPDFPageTapped(PDFVPage vpage)
        {
            goback(vpage.GetPageNo());
        }

        public System.Threading.Tasks.Task OnPDFAnnotRichMediaAsync(PDFAnnot annot)
        {
            return null;
        }

        //public void OnPDFZoomStart()
        //{
        //    //Do nothing
        //}

        //public void OnPDFZoomEnd()
        //{
        //    //Do nothing
        //}
    }
}
