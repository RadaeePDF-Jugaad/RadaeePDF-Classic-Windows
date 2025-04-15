using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Storage;

namespace com.radaee.reader
{
    class PDFGlobal
    {
        static private bool ms_loaded = false;
        static private void load_data()
        {
            if (ms_loaded) return;
            ms_loaded = true;
            String inst_path = Package.Current.InstalledLocation.Path;
            String cmap_path = inst_path + "\\Assets\\dat\\cmaps.dat";
            String umap_path = inst_path + "\\Assets\\dat\\umaps.dat";
            String cmyk_path = inst_path + "\\Assets\\dat\\cmyk_rgb.dat";
            if (new FileInfo(cmap_path).Exists && new FileInfo(umap_path).Exists)
                RDPDFLib.pdf.PDFGlobal.SetCMapsPath(cmap_path, umap_path);
            if (new FileInfo(cmyk_path).Exists)
                RDPDFLib.pdf.PDFGlobal.SetCMYKICC(cmyk_path);
            RDPDFLib.pdf.PDFGlobal.FontFileListStart();
            //the new UWP can access font directory in system path.
            String fpath = SystemDataPaths.GetDefault().Fonts;
            DirectoryInfo finfo = new DirectoryInfo(fpath);
            FileInfo[] files = finfo.GetFiles();
            foreach (FileInfo file in files)
            {
                String ext = file.Extension.ToLower();
                if (ext.CompareTo(".ttf") == 0 || ext.CompareTo(".ttc") == 0 ||
                    ext.CompareTo(".otf") == 0 || ext.CompareTo(".otc") == 0)
                    RDPDFLib.pdf.PDFGlobal.FontFileListAdd(file.FullName);
            }
            RDPDFLib.pdf.PDFGlobal.FontFileListAdd(inst_path + "\\Assets\\font\\argbsn00lp.ttf");
            RDPDFLib.pdf.PDFGlobal.FontFileListAdd(inst_path + "\\Assets\\font\\arimo.ttf");
            RDPDFLib.pdf.PDFGlobal.FontFileListAdd(inst_path + "\\Assets\\font\\arimob.ttf");
            RDPDFLib.pdf.PDFGlobal.FontFileListAdd(inst_path + "\\Assets\\font\\arimobi.ttf");
            RDPDFLib.pdf.PDFGlobal.FontFileListAdd(inst_path + "\\Assets\\font\\arimoi.ttf");
            RDPDFLib.pdf.PDFGlobal.FontFileListAdd(inst_path + "\\Assets\\font\\texgy.otf");
            RDPDFLib.pdf.PDFGlobal.FontFileListAdd(inst_path + "\\Assets\\font\\texgyb.otf");
            RDPDFLib.pdf.PDFGlobal.FontFileListAdd(inst_path + "\\Assets\\font\\texgybi.otf");
            RDPDFLib.pdf.PDFGlobal.FontFileListAdd(inst_path + "\\Assets\\font\\texgyi.otf");
            RDPDFLib.pdf.PDFGlobal.FontFileListAdd(inst_path + "\\Assets\\font\\cousine.ttf");
            RDPDFLib.pdf.PDFGlobal.FontFileListAdd(inst_path + "\\Assets\\font\\cousineb.ttf");
            RDPDFLib.pdf.PDFGlobal.FontFileListAdd(inst_path + "\\Assets\\font\\cousinei.ttf");
            RDPDFLib.pdf.PDFGlobal.FontFileListAdd(inst_path + "\\Assets\\font\\cousinebi.ttf");
            RDPDFLib.pdf.PDFGlobal.FontFileListAdd(inst_path + "\\Assets\\font\\symbol.ttf");
            RDPDFLib.pdf.PDFGlobal.FontFileListAdd(inst_path + "\\Assets\\font\\amiriRegular.ttf");
            RDPDFLib.pdf.PDFGlobal.FontFileListEnd();

            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Arial", "Arimo");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Arial Bold", "Arimo Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Arial BoldItalic", "Arimo Bold Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Arial Italic", "Arimo Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Arial,Bold", "Arimo Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Arial,BoldItalic", "Arimo Bold Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Arial,Italic", "Arimo Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Arial-Bold", "Arimo Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Arial-BoldItalic", "Arimo Bold Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Arial-Italic", "Arimo Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("ArialMT", "Arimo");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Calibri", "Arimo");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Calibri Bold", "Arimo Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Calibri BoldItalic", "Arimo Bold Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Calibri Italic", "Arimo Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Calibri,Bold", "Arimo Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Calibri,BoldItalic", "Arimo Bold Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Calibri,Italic", "Arimo Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Calibri-Bold", "Arimo Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Calibri-BoldItalic", "Arimo Bold Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Calibri-Italic", "Arimo Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Helvetica", "Arimo");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Helvetica Bold", "Arimo Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Helvetica BoldItalic", "Arimo Bold Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Helvetica Italic", "Arimo Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Helvetica,Bold", "Arimo,Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Helvetica,BoldItalic", "Arimo Bold Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Helvetica,Italic", "Arimo Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Helvetica-Bold", "Arimo Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Helvetica-BoldItalic", "Arimo Bold Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Helvetica-Italic", "Arimo Italic");

            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Garamond", "TeXGyreTermes-Regular");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Garamond,Bold", "TeXGyreTermes-Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Garamond,BoldItalic", "TeXGyreTermes-BoldItalic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Garamond,Italic", "TeXGyreTermes-Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Garamond-Bold", "TeXGyreTermes-Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Garamond-BoldItalic", "TeXGyreTermes-BoldItalic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Garamond-Italic", "TeXGyreTermes-Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Times", "TeXGyreTermes-Regular");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Times,Bold", "TeXGyreTermes-Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Times,BoldItalic", "TeXGyreTermes-BoldItalic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Times,Italic", "TeXGyreTermes-Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Times-Bold", "TeXGyreTermes-Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Times-BoldItalic", "TeXGyreTermes-BoldItalic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Times-Italic", "TeXGyreTermes-Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Times-Roman", "TeXGyreTermes-Regular");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Times New Roman", "TeXGyreTermes-Regular");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Times New Roman,Bold", "TeXGyreTermes-Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Times New Roman,BoldItalic", "TeXGyreTermes-BoldItalic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Times New Roman,Italic", "TeXGyreTermes-Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Times New Roman-Bold", "TeXGyreTermes-Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Times New Roman-BoldItalic", "TeXGyreTermes-BoldItalic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Times New Roman-Italic", "TeXGyreTermes-Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRoman", "TeXGyreTermes-Regular");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRoman,Bold", "TeXGyreTermes-Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRoman,BoldItalic", "TeXGyreTermes-BoldItalic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRoman,Italic", "TeXGyreTermes-Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRoman-Bold", "TeXGyreTermes-Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRoman-BoldItalic", "TeXGyreTermes-BoldItalic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRoman-Italic", "TeXGyreTermes-Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRomanPS", "TeXGyreTermes-Regular");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRomanPS,Bold", "TeXGyreTermes-Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRomanPS,BoldItalic", "TeXGyreTermes-BoldItalic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRomanPS,Italic", "TeXGyreTermes-Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRomanPS-Bold", "TeXGyreTermes-Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRomanPS-BoldItalic", "TeXGyreTermes-BoldItalic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRomanPS-Italic", "TeXGyreTermes-Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRomanPSMT", "TeXGyreTermes-Regular");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRomanPSMT,Bold", "TeXGyreTermes-Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRomanPSMT,BoldItalic", "TeXGyreTermes-BoldItalic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRomanPSMT,Italic", "TeXGyreTermes-Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRomanPSMT-Bold", "TeXGyreTermes-Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRomanPSMT-BoldItalic", "TeXGyreTermes-BoldItalic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("TimesNewRomanPSMT-Italic", "TeXGyreTermes-Italic");

            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier", "Cousine");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier Bold", "Cousine Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier BoldItalic", "Cousine Bold Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier Italic", "Cousine Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier,Bold", "Cousine Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier,BoldItalic", "Cousine Bold Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier,Italic", "Cousine Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier-Bold", "Cousine Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier-BoldItalic", "Cousine Bold Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier-Italic", "Cousine Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier New", "Cousine");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier New Bold", "Cousine Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier New BoldItalic", "Cousine Bold Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier New Italic", "Cousine Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier New,Bold", "Cousine Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier New,BoldItalic", "Cousine Bold Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier New,Italic", "Cousine Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier New-Bold", "Cousine Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier New-BoldItalic", "Cousine Bold Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Courier New-Italic", "Cousine Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("CourierNew", "Cousine");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("CourierNew Bold", "Cousine Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("CourierNew BoldItalic", "Cousine Bold Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("CourierNew Italic", "Cousine Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("CourierNew,Bold", "Cousine Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("CourierNew,BoldItalic", "Cousine Bold Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("CourierNew,Italic", "Cousine Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("CourierNew-Bold", "Cousine Bold");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("CourierNew-BoldItalic", "Cousine Bold Italic");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("CourierNew-Italic", "Cousine Italic");

            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Symbol", "Symbol Neu for Powerline");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Symbol,Bold", "Symbol Neu for Powerline");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Symbol,BoldItalic", "Symbol Neu for Powerline");
            RDPDFLib.pdf.PDFGlobal.FontFileMapping("Symbol,Italic", "Symbol Neu for Powerline");

            int face_first = 0;
            int face_count = RDPDFLib.pdf.PDFGlobal.GetFaceCount();
            String rand_fname = null;
            uint sys_fonts = 0;
            while (face_first < face_count)
            {
                String fname = RDPDFLib.pdf.PDFGlobal.GetFaceName(face_first);
                if (fname != null)
                {
                    if (fname.CompareTo("SimSun") == 0) sys_fonts |= 1;
                    if (fname.CompareTo("Microsoft JHengHei") == 0 || fname.CompareTo("MingLiU") == 0) sys_fonts |= 2;
                    if (fname.CompareTo("MS Gothic") == 0) sys_fonts |= 4;
                    if (fname.CompareTo("Malgun Gothic Regular") == 0) sys_fonts |= 8;
                }
                if (rand_fname == null && fname != null && fname.Length > 0)
                    rand_fname = fname;
                face_first++;
            }
            // set default fonts.
            if (sys_fonts > 0)
            {
                RDPDFLib.pdf.PDFGlobal.SetDefaultFont("", "Calibri", true);
                RDPDFLib.pdf.PDFGlobal.SetDefaultFont("", "Times New Roman", false);
                RDPDFLib.pdf.PDFGlobal.SetDefaultFont("GB1", "SimSun", true);
                RDPDFLib.pdf.PDFGlobal.SetDefaultFont("GB1", "SimSun", false);
                if (!RDPDFLib.pdf.PDFGlobal.SetDefaultFont("CNS1", "Microsoft JHengHei", true))
                    RDPDFLib.pdf.PDFGlobal.SetDefaultFont("CNS1", "MingLiU", true);
                if (!RDPDFLib.pdf.PDFGlobal.SetDefaultFont("CNS1", "Microsoft JHengHei", false))
                    RDPDFLib.pdf.PDFGlobal.SetDefaultFont("CNS1", "MingLiU", false);
                RDPDFLib.pdf.PDFGlobal.SetDefaultFont("Japan1", "MS Gothic", true);
                RDPDFLib.pdf.PDFGlobal.SetDefaultFont("Japan1", "MS Gothic", false);
                RDPDFLib.pdf.PDFGlobal.SetDefaultFont("Korea1", "Malgun Gothic Regular", true);
                RDPDFLib.pdf.PDFGlobal.SetDefaultFont("Korea1", "Malgun Gothic Regular", false);
                RDPDFLib.pdf.PDFGlobal.SetAnnotFont("SimSun");
            }
            else
            {
                if (!RDPDFLib.pdf.PDFGlobal.SetDefaultFont("", "AR PL SungtiL GB", true) && rand_fname != null)
                    RDPDFLib.pdf.PDFGlobal.SetDefaultFont("", rand_fname, true);
                if (!RDPDFLib.pdf.PDFGlobal.SetDefaultFont("", "AR PL SungtiL GB", false) && rand_fname != null)
                    RDPDFLib.pdf.PDFGlobal.SetDefaultFont("", rand_fname, false);
                if (!RDPDFLib.pdf.PDFGlobal.SetAnnotFont("AR PL SungtiL GB") && rand_fname != null)
                    RDPDFLib.pdf.PDFGlobal.SetAnnotFont(rand_fname);
            }

            RDPDFLib.pdf.PDFGlobal.LoadStdFont(13, inst_path + "\\Assets\\font\\rdf013");
        }
        static public bool init()
        {
            load_data();
            //binding to package "2625.RadaeePDFReader", can active version before: 2025-05-09
            //RDPDFLib.pdf.PDFGlobal.GetVersion() can get version string.
            int licenseType = RDPDFLib.pdf.PDFGlobal.Active("52DE4976539BFCE23ABE988B73730A8F88C6CB9CAB13C76C92C6D3D28E717DD0224478233438C10FD5321E85E57E68FE");
            //int licenseType = RDPDFLib.pdf.PDFGlobal.Active("radaee", "support@radaeepdf.com", "YOOW28-VS57CA-9ZOU9E-OQ31K2-5R5V9L-KM0Y1L");
            return (licenseType > 0);
        }
    }
}
