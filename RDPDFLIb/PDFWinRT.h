#ifndef _PDF_WINDOWS_RT_
#define _PDF_WINDOWS_RT_
#include <windows.h>
#include <collection.h>
#include <ppltasks.h>
using namespace Platform;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::Graphics::Imaging;

#ifdef __cplusplus
class IPDFStream
{
public:
	virtual bool Writeable() const = 0;
	virtual unsigned long long GetLen() const = 0;
	virtual unsigned long long GetPos() const = 0;
	virtual bool SetPos( unsigned long long off ) = 0;
	virtual unsigned int Read( void *pBuf, unsigned int dwBuf ) = 0;
	virtual unsigned int Write( const void *pBuf, unsigned int dwBuf ) = 0;
	virtual void Close() = 0;
	virtual void Flush() = 0;
};

class IPDFJSDelegate
{
public:
	virtual void OnConsole(int cmd, const char *para) = 0;
	virtual int OnAlert(int btn, const char *msg, const char *title) = 0;
	virtual bool OnDocClose() = 0;
	virtual char *OnTmpFile() = 0;
	virtual void OnUncaughtException(int code, const char *msg) = 0;
};

extern "C" {
#endif
typedef enum
{
	err_ok,
	err_invalid_para,
    err_open,
    err_password,
    err_encrypt,
    err_bad_file,
}PDF_ERR;
typedef enum
{
    mode_poor = 0,
    mode_normal = 1,
    mode_best = 2,
}PDF_RENDER_MODE;
typedef struct
{
    float x;
    float y;
}PDF_POINT;
typedef struct
{
    float left;
    float top;
    float right;
    float bottom;
}PDF_RECT;
typedef struct _PDFDIB * PDF_DIB;
typedef struct _PDF_MATRIX * PDF_MATRIX;
typedef struct _PDF_DOC * PDF_DOC;
typedef struct _PDF_OUTLINE * PDF_OUTLINE;
typedef struct _PDF_PAGE *PDF_PAGE;
typedef struct _PDF_FINDER * PDF_FINDER;
typedef struct _PDF_ANNOT * PDF_ANNOT;
typedef struct _PDF_INK * PDF_INK;
typedef struct _PDF_BMP * PDF_BMP;
typedef struct _PDF_IMPORTCTX * PDF_IMPORTCTX;

typedef struct _PDF_PATH * PDF_PATH;
typedef struct _PDF_PAGECONTENT * PDF_PAGECONTENT;
typedef struct _PDF_DOC_FONT * PDF_DOC_FONT;
typedef struct _PDF_PAGE_FONT * PDF_PAGE_FONT;
typedef struct _PDF_DOC_GSTATE * PDF_DOC_GSTATE;
typedef struct _PDF_PAGE_GSTATE * PDF_PAGE_GSTATE;
typedef struct _PDF_DOC_IMAGE * PDF_DOC_IMAGE;
typedef struct _PDF_PAGE_IMAGE * PDF_PAGE_IMAGE;
typedef struct _PDF_DOC_FORM * PDF_DOC_FORM;
typedef struct _PDF_PAGE_FORM * PDF_PAGE_FORM;
typedef struct _PDF_OBJ * PDF_OBJ;
typedef unsigned long long PDF_OBJ_REF;
typedef struct _PDF_SIGN *PDF_SIGN;

void Global_getVersion(char* sver);
int Global_active(String ^ serial);
//bool Global_activeStandard(const char* company, const char* mail, const char* serial);
//bool Global_activeProfession(const char* company, const char* mail, const char* serial);
//bool Global_activePremium(const char* company, const char* mail, const char* serial);


//load starnard font from path
void Global_loadStdFont( int index, const char *path );
//unload standard font
void Global_unloadStdFont( int index );
//set cmaps path
void Global_setCMapsPath( const char *cmaps, const char *umaps );
bool Global_setCMYKICC(const char *path);
//create font list
void Global_fontfileListStart();
//add a true type font file to font list
void Global_fontfileListAdd( const char *font_file );
//end font list
void Global_fontfileListEnd();
bool Global_fontfileMapping( const char *map_name, const char *name );
//set default font
bool Global_setDefaultFont( const char *collection, const char *font_name, bool fixed );
//set annotation font for editing mostly for edit-box/combo-box
bool Global_setAnnotFont( const char *font_name );
int Global_getFaceCount();
const char *Global_getFaceName( int index );
//create a DIB object
PDF_DIB Global_dibGet( PDF_DIB dib, int width, int height );
//free DIB object.
void Global_dibFree( PDF_DIB dib );
void Global_toDIBPoint( PDF_MATRIX matrix, const PDF_POINT *ppoint, PDF_POINT *dpoint );
void Global_toPDFPoint( PDF_MATRIX matrix, const PDF_POINT *dpoint, PDF_POINT *ppoint );
void Global_toDIBRect( PDF_MATRIX matrix, const PDF_RECT *prect, PDF_RECT *drect );
void Global_toPDFRect( PDF_MATRIX matrix, const PDF_RECT *drect, PDF_RECT *prect );
//get pixels data from DIB object
void *Global_dibGetData( PDF_DIB dib );
int Global_dibGetWidth( PDF_DIB dib );
int Global_dibGetHeight( PDF_DIB dib );
bool Global_dibSaveJPG(PDF_DIB dib, const char *path, int quality);
void Global_drawScroll(WriteableBitmap^dst, PDF_DIB dib1, PDF_DIB dib2, int x, int y, int style, unsigned int back_side_clr);
bool Global_drawAnnotIcon(int annot_type, int icon, WriteableBitmap ^ dib);
bool Global_drawDashLine(const float* dash, int dash_cnt, WriteableBitmap^ dib);
bool Global_drawLineHead(int head, WriteableBitmap^ dib);
PDF_BMP Global_lockBitmap( WriteableBitmap ^dst );
PDF_BMP Global_lockSoftBitmap(SoftwareBitmap^ dst);
bool Global_saveBitmapJPG(PDF_BMP data, const char *path, int quality);
void Global_unlockBitmap(PDF_BMP data);
void Global_scaleDIB( PDF_DIB dst, PDF_DIB dib );
void Global_drawDIB( PDF_BMP dst, PDF_DIB dib, int x, int y );
void Global_drawRect( PDF_BMP dst, int color, int x, int y, int w, int h, int mode );
void Global_eraseColor( PDF_BMP dst, int color );
void Global_setAnnotTransparency( int color );

//create a matrix object
PDF_MATRIX Matrix_create( float xx, float yx, float xy, float yy, float x0, float y0 );
PDF_MATRIX Matrix_createScale( float scalex, float scaley, float x0, float y0 );
void Matrix_invert( PDF_MATRIX matrix );
void Matrix_transformPath( PDF_MATRIX matrix, PDF_PATH path );
void Matrix_transformInk( PDF_MATRIX matrix, PDF_INK ink );
void Matrix_transformRect( PDF_MATRIX matrix, PDF_RECT *rect );
void Matrix_transformPoint( PDF_MATRIX matrix, PDF_POINT *point );
void Matrix_destroy( PDF_MATRIX matrix );

//create a Ink object
PDF_INK Ink_create( float line_w, int color );
void Ink_destroy( PDF_INK ink );
void Ink_onDown( PDF_INK ink, float x, float y );
void Ink_onMove( PDF_INK ink, float x, float y );
void Ink_onUp( PDF_INK ink, float x, float y );
int Ink_getNodeCount( PDF_INK ink );
int Ink_getNode( PDF_INK hand, int index, PDF_POINT *pt );

void Document_setOpenFlag(int flag);
PDF_DOC Document_openPath( const char *path, const char *password, PDF_ERR *err );
PDF_DOC Document_openPathW(const wchar_t* path, const char* password, PDF_ERR* err);
PDF_DOC Document_open( IRandomAccessStream ^stream, const char *password, PDF_ERR *err );
PDF_DOC Document_openStream( IPDFStream *stream, const char *password, PDF_ERR *err );
PDF_DOC Document_create( IRandomAccessStream ^stream, PDF_ERR *err );
PDF_DOC Document_createForStream( IPDFStream *stream, PDF_ERR *err );
PDF_DOC Document_createForPath( const char *path, PDF_ERR *err );
bool Document_setCache( PDF_DOC doc, const char *path );
String^ Document_getXMP(PDF_DOC doc);
bool Document_setXMP(PDF_DOC doc, String^ xmp);
bool Document_runJS( PDF_DOC doc, const char *js, IPDFJSDelegate *del);
int Document_getPermission( PDF_DOC doc );
int Document_getPerm( PDF_DOC doc );
int Document_getEFCount(PDF_DOC doc);
String^ Document_getEFName(PDF_DOC doc, int index);
String^ Document_getEFDesc(PDF_DOC doc, int index);
bool Document_getEFData(PDF_DOC doc, int index, String^ path);
int Document_getJSCount(PDF_DOC doc);
String^ Document_getJSName(PDF_DOC doc, int index);
String^ Document_getJS(PDF_DOC doc, int index);
bool Document_exportForm(PDF_DOC doc, char* str, int len);
bool Document_canSave( PDF_DOC doc );
int Document_getLinearizedStatus(PDF_DOC doc);

String ^Document_getOutlineLabel(PDF_DOC doc, PDF_OUTLINE outlinenode);
int Document_getOutlineDest( PDF_DOC doc, PDF_OUTLINE outlinenode );
void Document_getOutlineDest2(PDF_DOC doc, PDF_OUTLINE outlinenode, int* vals);
String^ Document_getOutlineFileLink(PDF_DOC doc, PDF_OUTLINE outlinenode);
PDF_OUTLINE Document_getOutlineChild(PDF_DOC doc, PDF_OUTLINE outlinenode);
PDF_OUTLINE Document_getOutlineNext(PDF_DOC doc, PDF_OUTLINE outlinenode);
bool Document_addOutlineChild(PDF_DOC doc, PDF_OUTLINE outlinenode, const wchar_t *label, int pageno, float top);
bool Document_addOutlineNext(PDF_DOC doc, PDF_OUTLINE outlinenode, const wchar_t *label, int pageno, float top);
bool Document_removeOutline(PDF_DOC doc, PDF_OUTLINE outlinenode);

String^ Document_getMeta(PDF_DOC doc, const char* tag);
bool Document_setMeta(PDF_DOC doc, const char* tag, const wchar_t* meta);
PDF_POINT Document_getPagesMaxSize(PDF_DOC doc);
float Document_getPageWidth( PDF_DOC doc, int pageno );
float Document_getPageHeight( PDF_DOC doc, int pageno );
int Document_getPageCount( PDF_DOC doc );
bool Document_getPageLabel(PDF_DOC doc, int pageno, wchar_t *wtxt, int wlen);
bool Document_setPageRotate(PDF_DOC doc, int pageno, int degree);
bool Document_changePageRect(PDF_DOC doc, int pageno, float dl, float dt, float dr, float db);
bool Document_save( PDF_DOC doc );
bool Document_saveAs( PDF_DOC doc, const char *dst );
bool Document_saveAsW(PDF_DOC doc, const wchar_t* dst);
bool Document_encryptAsW(PDF_DOC doc, const wchar_t* dst, const wchar_t* upswd, const wchar_t* opswd, int perm, int method, const BYTE* id);
bool Document_isEncrypted( PDF_DOC doc );
int Document_verifySign(PDF_DOC doc, PDF_SIGN sign);
void Document_close( PDF_DOC doc );
PDF_PAGE Document_getPage( PDF_DOC doc, int pageno );
PDF_DOC_FONT Document_newFontCID( PDF_DOC doc, const char *name, int style );
float Document_getFontAscent( PDF_DOC doc, PDF_DOC_FONT font );
float Document_getFontDescent( PDF_DOC doc, PDF_DOC_FONT font );
PDF_DOC_GSTATE Document_newGState( PDF_DOC doc );
bool Document_setGStateStrokeAlpha( PDF_DOC doc, PDF_DOC_GSTATE state, int alpha );
bool Document_setGStateFillAlpha( PDF_DOC doc, PDF_DOC_GSTATE state, int alpha );
bool Document_setGStateStrokeDash(PDF_DOC doc, PDF_DOC_GSTATE state, const float *dash, int dash_cnt, float phase);
bool Document_setGStateBlendMode(PDF_DOC doc, PDF_DOC_GSTATE state, int bmode);

PDF_DOC_FORM Document_newForm(PDF_DOC doc);
PDF_PAGE_FONT Document_addFormResFont(PDF_DOC doc, PDF_DOC_FORM form, PDF_DOC_FONT font);
PDF_PAGE_IMAGE Document_addFormResImage(PDF_DOC doc, PDF_DOC_FORM form, PDF_DOC_IMAGE image);
PDF_PAGE_GSTATE Document_addFormResGState(PDF_DOC doc, PDF_DOC_FORM form, PDF_DOC_GSTATE gstate);
PDF_PAGE_FORM Document_addFormResForm(PDF_DOC doc, PDF_DOC_FORM form, PDF_DOC_FORM sub);
void Document_setFormContent(PDF_DOC doc, PDF_DOC_FORM form, float x, float y, float w, float h, PDF_PAGECONTENT content);
void Document_setFormTransparency(PDF_DOC doc, PDF_DOC_FORM form, bool isolate, bool knockout);
void Document_freeForm(PDF_DOC doc, PDF_DOC_FORM form);

PDF_PAGE Document_newPage( PDF_DOC doc, int pageno, float w, float h );
bool Document_removePage( PDF_DOC doc, int pageno );
PDF_IMPORTCTX Document_importStart( PDF_DOC doc, PDF_DOC doc_src );
bool Document_importPage( PDF_DOC doc, PDF_IMPORTCTX ctx, int srcno, int dstno );
void Document_importEnd( PDF_DOC doc, PDF_IMPORTCTX ctx );
bool Document_movePage( PDF_DOC doc, int pageno1, int pageno2 );
PDF_DOC_IMAGE Document_newImage(PDF_DOC doc, WriteableBitmap ^bitmap, bool has_alpha);
PDF_DOC_IMAGE Document_newImage2(PDF_DOC doc, SoftwareBitmap^ bitmap, bool has_alpha);
PDF_DOC_IMAGE Document_newImageMatte(PDF_DOC doc, WriteableBitmap^ bitmap, unsigned int matte);
PDF_DOC_IMAGE Document_newImage2Matte(PDF_DOC doc, SoftwareBitmap^ bitmap, unsigned int matte);
PDF_DOC_IMAGE Document_newImageJPEG( PDF_DOC doc, const char *path );
PDF_DOC_IMAGE Document_newImageJPX( PDF_DOC doc, const char *path );


String ^Sign_getIssue(PDF_SIGN sign);
String ^Sign_getSubject(PDF_SIGN sign);
long Sign_getVersion(PDF_SIGN sign);
String^ Sign_getName(PDF_SIGN sign);
String^ Sign_getLocation(PDF_SIGN sign);
String^ Sign_getReason(PDF_SIGN sign);
String^ Sign_getContact(PDF_SIGN sign);
String^ Sign_getModDT(PDF_SIGN sign);

int Page_sign(PDF_PAGE page, PDF_DOC_FORM appearence, const PDF_RECT *box, const char *cert_file, const char *pswd, const char *name, const char *reason, const char *location, const char *contact);
bool Page_getCropBox( PDF_PAGE page, PDF_RECT *box );
bool Page_getMediaBox( PDF_PAGE page, PDF_RECT *box );
void Page_close( PDF_PAGE page );

float Page_reflowStart(PDF_PAGE page, float width, float ratio, bool reflow_images);
float Page_reflowStart2(PDF_PAGE page, float width, float ratio, bool reflow_images, const char* font_name);
bool Page_reflow(PDF_PAGE page, PDF_DIB dib, float orgx, float orgy);
int Page_reflowGetParaCount(PDF_PAGE page);
int Page_reflowGetCharCount(long page, int iparagraph);
float Page_reflowGetCharWidth(PDF_PAGE page, int iparagraph, int ichar);
float Page_reflowGetCharHeight(PDF_PAGE page, int iparagraph, int ichar);
int Page_reflowGetCharColor(PDF_PAGE page, int iparagraph, int ichar);
int Page_reflowGetCharUnicode(PDF_PAGE page, int iparagraph, int ichar);
PDF_RECT Page_reflowGetCharRect(PDF_PAGE page, int iparagraph, int ichar, int orgx, int orgy);
String ^Page_reflowGetText(PDF_PAGE page, int iparagraph1, int ichar1, int iparagraph2, int ichar2);
bool Page_reflowToBmp(PDF_PAGE page, PDF_BMP bitmap, float orgx, float orgy);

void Page_renderPrepare( PDF_PAGE page, PDF_DIB dib );
bool Page_render( PDF_PAGE page, PDF_DIB dib, PDF_MATRIX matrix, bool show_annots, PDF_RENDER_MODE mode );
bool Page_renderToBmp( PDF_PAGE page, PDF_BMP bitmap, PDF_MATRIX matrix, bool show_annots, PDF_RENDER_MODE mode );
bool Page_renderAnnotToBmp(PDF_PAGE page, PDF_ANNOT annot, PDF_BMP bitmap);
void Page_renderCancel( PDF_PAGE page );
bool Page_renderIsFinished( PDF_PAGE page );
void Page_objsStart( PDF_PAGE page );
int Page_objsGetCharIndex( PDF_PAGE page, float x, float y );
int Page_objsGetCharCount( PDF_PAGE page );
int Page_objsGetString( PDF_PAGE page, int from, int to, char *buf, int len );
int Page_objsGetStringW( PDF_PAGE page, int from, int to, wchar_t *buf, int len );
void Page_objsGetCharRect( PDF_PAGE page, int index, PDF_RECT *rect );
const char *Page_objsGetCharFontName( PDF_PAGE page, int index );
int Page_objsAlignWord( PDF_PAGE page, int from, int dir );
PDF_FINDER Page_findOpenW( PDF_PAGE page, const wchar_t *str, bool match_case, bool whole_word );
PDF_FINDER Page_findOpen2W(PDF_PAGE page, const wchar_t* str, bool match_case, bool whole_word, bool skip_blanks);
int Page_findGetCount( PDF_FINDER finder );
int Page_findGetFirstChar( PDF_FINDER finder, int index );
int Page_findGetEndChar(PDF_FINDER finder, int index);
void Page_findClose( PDF_FINDER finder );
int Page_getRotate(PDF_PAGE page);
int Page_getAnnotCount( PDF_PAGE page );
PDF_ANNOT Page_getAnnot( PDF_PAGE page, int index );
PDF_ANNOT Page_getAnnotFromPoint( PDF_PAGE page, float x, float y );
int Page_getAnnotSignStatus(PDF_PAGE page, PDF_ANNOT annot);
PDF_SIGN Page_getAnnotSign(PDF_PAGE page, PDF_ANNOT annot);

bool Page_isAnnotLocked( PDF_PAGE page, PDF_ANNOT annot );
void Page_setAnnotLock(PDF_PAGE page, PDF_ANNOT annot, bool lock);
bool Page_isAnnotLockedContent( PDF_PAGE page, PDF_ANNOT annot );
bool Page_isAnnotHide( PDF_PAGE page, PDF_ANNOT annot );
void Page_setAnnotHide( PDF_PAGE page, PDF_ANNOT annot, bool hide );

/**
	* get annotation type.<br/>
	* this can be invoked after ObjsStart or Render or RenderToBmp.<br/>
	* this method valid in professional or premium version
	* @return type as these values:<br/>
	* 0:  unknown<br/>
	* 1:  text<br/>
	* 2:  link<br/>
	* 3:  free text<br/>
	* 4:  line<br/>
	* 5:  square<br/>
	* 6:  circle<br/>
	* 7:  polygon<br/>
	* 8:  polyline<br/>
	* 9:  text hilight<br/>
	* 10: text under line<br/>
	* 11: text squiggly<br/>
	* 12: text strikeout<br/>
	* 13: stamp<br/>
	* 14: caret<br/>
	* 15: ink<br/>
	* 16: popup<br/>
	* 17: file attachment<br/>
	* 18: sound<br/>
	* 19: movie<br/>
	* 20: widget<br/>
	* 21: screen<br/>
	* 22: print mark<br/>
	* 23: trap net<br/>
	* 24: water mark<br/>
	* 25: 3d object<br/>
	* 26: rich media
	*/
int Page_getAnnotType( PDF_PAGE page, PDF_ANNOT annot );
int Page_signAnnotField(PDF_PAGE page, PDF_ANNOT annot, PDF_DOC_FORM appearence, const char *cert_file, const char *pswd, const char* name, const char *reason, const char *location, const char *contact);

/**
	* get annotation field type in acroForm.<br/>
	* this can be invoked after ObjsStart or Render or RenderToBmp.<br/>
	* this method valid in premium version
	* @return type as these values:<br/>
	* 0: unknown<br/>
	* 1: button field<br/>
	* 2: text field<br/>
	* 3: choice field<br/>
	* 4: signature field<br/>
	*/
int Page_getAnnotFieldType( PDF_PAGE page, PDF_ANNOT annot );
int Page_getAnnotFieldNameW( PDF_PAGE page, PDF_ANNOT annot, wchar_t *buf, int len );
int Page_getAnnotFieldNameWithNOW(PDF_PAGE page, PDF_ANNOT annot, wchar_t *buf, int len);
int Page_getAnnotFieldFullNameW( PDF_PAGE page, PDF_ANNOT annot, wchar_t *buf, int len );
int Page_getAnnotFieldFullName2W( PDF_PAGE page, PDF_ANNOT annot, wchar_t *buf, int len );
String ^Page_getAnnotFieldJS(PDF_PAGE page, PDF_ANNOT annot, int idx);
bool Page_isAnnotReadOnly(PDF_PAGE page, PDF_ANNOT annot);
void Page_setAnnotReadOnly(PDF_PAGE page, PDF_ANNOT annot, bool lock);
void Page_getAnnotRect( PDF_PAGE page, PDF_ANNOT annot, PDF_RECT *rect );
void Page_setAnnotRect( PDF_PAGE page, PDF_ANNOT annot, const PDF_RECT *rect );

PDF_PATH Page_getAnnotInkPath(PDF_PAGE page, PDF_ANNOT annot);
bool Page_setAnnotInkPath(PDF_PAGE page, PDF_ANNOT annot, PDF_PATH path);
int Page_getAnnotFillColor( PDF_PAGE page, PDF_ANNOT annot );
bool Page_setAnnotFillColor( PDF_PAGE page, PDF_ANNOT annot, int color );
int Page_getAnnotStrokeColor( PDF_PAGE page, PDF_ANNOT annot );
bool Page_setAnnotStrokeColor( PDF_PAGE page, PDF_ANNOT annot, int color );
float Page_getAnnotStrokeWidth( PDF_PAGE page, PDF_ANNOT annot );
bool Page_setAnnotStrokeWidth( PDF_PAGE page, PDF_ANNOT annot, float width );
int Page_getAnnotStrokeDash(PDF_PAGE page, PDF_ANNOT annot, float* dash, int max);
bool Page_setAnnotStrokeDash(PDF_PAGE page, PDF_ANNOT annot, const float *dash, int cnt);
/**
	* set icon for sticky text note/file attachment annotation.<br/>
	* this can be invoked after ObjsStart or Render or RenderToBmp.<br/>
	* you need render page again to show modified annotation.<br/>
	* this method valid in professional or premium version
	* @param icon icon value depends on annotation type.<br/>
	* <strong>For sticky text note:</strong><br/>
	* 0: Note<br/>
	* 1: Comment<br/>
	* 2: Key<br/>
	* 3: Help<br/>
	* 4: NewParagraph<br/>
	* 5: Paragraph<br/>
	* 6: Insert<br/>
	* 7: Check<br/>
	* 8: Circle<br/>
	* 9: Cross<br/>
	* <strong>For file attachment:</strong><br/>
	* 0: PushPin<br/>
	* 1: Graph<br/>
	* 2: Paperclip<br/>
	* 3: Tag<br/>
	* @return true or false.
	*/
bool Page_setAnnotIcon( PDF_PAGE page, PDF_ANNOT annot, int icon );
/**
	* get icon value for sticky text note/file attachment annotation.<br/>
	* this can be invoked after ObjsStart or Render or RenderToBmp.<br/>
	* this method valid in professional or premium version
	* @return icon value depends on annotation type.<br/>
	* <strong>For sticky text note:</strong><br/>
	* 0: Note<br/>
	* 1: Comment<br/>
	* 2: Key<br/>
	* 3: Help<br/>
	* 4: NewParagraph<br/>
	* 5: Paragraph<br/>
	* 6: Insert<br/>
	* 7: Check<br/>
	* 8: Circle<br/>
	* 9: Cross<br/>
	* <strong>For file attachment:</strong><br/>
	* 0: PushPin<br/>
	* 1: Graph<br/>
	* 2: Paperclip<br/>
	* 3: Tag<br/>
	*/
int Page_getAnnotIcon( PDF_PAGE page, PDF_ANNOT annot );

int Page_getAnnotDest( PDF_PAGE page, PDF_ANNOT annot );
String ^Page_getAnnotJS(PDF_PAGE page, PDF_ANNOT annot);
String ^Page_getAnnotURI(PDF_PAGE page, PDF_ANNOT annot);
String^ Page_getAnnot3D( PDF_PAGE page, PDF_ANNOT annot);
String^ Page_getAnnotMovie(PDF_PAGE page, PDF_ANNOT annot);
String^ Page_getAnnotSound(PDF_PAGE page, PDF_ANNOT annot);
String^ Page_getAnnotAttachment(PDF_PAGE page, PDF_ANNOT annot);
String^ Page_getAnnotRendition(PDF_PAGE page, PDF_ANNOT annot);
bool Page_getAnnot3DData( PDF_PAGE page, PDF_ANNOT annot, const char *path );
bool Page_getAnnotMovieData( PDF_PAGE page, PDF_ANNOT annot, const char *path );
bool Page_getAnnotSoundData( PDF_PAGE page, PDF_ANNOT annot, int *paras, const char *path );
bool Page_getAnnotAttachmentData( PDF_PAGE page, PDF_ANNOT annot, const char *path );
bool Page_getAnnotRenditionData(PDF_PAGE page, PDF_ANNOT annot, const char* path);
int Page_getAnnotRichMediaItemCount(PDF_PAGE page, PDF_ANNOT annot);
int Page_getAnnotRichMediaItemActived(PDF_PAGE page, PDF_ANNOT annot);
int Page_getAnnotRichMediaItemType(PDF_PAGE page, PDF_ANNOT annot, int idx);
String ^Page_getAnnotRichMediaItemAsset(PDF_PAGE page, PDF_ANNOT annot, int idx);
String ^Page_getAnnotRichMediaItemPara(PDF_PAGE page, PDF_ANNOT annot, int idx);
String ^Page_getAnnotRichMediaItemSource(PDF_PAGE page, PDF_ANNOT annot, int idx);
bool Page_getAnnotRichMediaItemSourceData(PDF_PAGE page, PDF_ANNOT annot, int idx, String ^save_path);
bool Page_getAnnotRichMediaData(PDF_PAGE page, PDF_ANNOT annot, String ^asset, String ^save_path);

String^ Page_getAnnotFileLink(PDF_PAGE page, PDF_ANNOT annot);
String^ Page_getAnnotRemoteDest(PDF_PAGE page, PDF_ANNOT annot);
PDF_ANNOT Page_getAnnotPopup(PDF_PAGE page, PDF_ANNOT annot);
bool Page_getAnnotPopupOpen(PDF_PAGE page, PDF_ANNOT annot);
bool Page_setAnnotPopupOpen(PDF_PAGE page, PDF_ANNOT annot, bool open);
int Page_getAnnotReplyCount(PDF_PAGE page, PDF_ANNOT annot);
PDF_ANNOT Page_getAnnotReply(PDF_PAGE page, PDF_ANNOT annot, int idx);
String^ Page_getAnnotPopupSubject(PDF_PAGE page, PDF_ANNOT annot);
String^ Page_getAnnotPopupText(PDF_PAGE page, PDF_ANNOT annot);
String^ Page_getAnnotPopupLabel(PDF_PAGE page, PDF_ANNOT annot);
bool Page_setAnnotPopupSubjectW( PDF_PAGE page, PDF_ANNOT annot, const wchar_t *subj );
bool Page_setAnnotPopupTextW( PDF_PAGE page, PDF_ANNOT annot, const wchar_t *text );
bool Page_setAnnotPopupLabelW(PDF_PAGE page, PDF_ANNOT annot, const wchar_t *text);
PDF_PATH Page_getAnnotPolygonPath(PDF_PAGE page, PDF_ANNOT annot);
bool Page_setAnnotPolygonPath(PDF_PAGE page, PDF_ANNOT annot, PDF_PATH path);
PDF_PATH Page_getAnnotPolylinePath(PDF_PAGE page, PDF_ANNOT annot);
bool Page_setAnnotPolylinePath(PDF_PAGE page, PDF_ANNOT annot, PDF_PATH path);
bool Page_getAnnotLinePoint(PDF_PAGE page, PDF_ANNOT annot, int idx, PDF_POINT* pt);
bool Page_setAnnotLinePoint(PDF_PAGE page, PDF_ANNOT annot, float x1, float y1, float x2, float y2);
int Page_getAnnotLineStyle(PDF_PAGE page, PDF_ANNOT annot);
bool Page_setAnnotLineStyle(PDF_PAGE page, PDF_ANNOT annot, int style);

int Page_getAnnotEditType( PDF_PAGE page, PDF_ANNOT annot );
bool Page_getAnnotEditTextRect( PDF_PAGE page, PDF_ANNOT annot, PDF_RECT *rect );
float Page_getAnnotEditTextSize( PDF_PAGE page, PDF_ANNOT annot );
bool Page_setAnnotEditTextSize(PDF_PAGE page, PDF_ANNOT annot, float fsize);
int Page_getAnnotEditTextAlign(PDF_PAGE page, PDF_ANNOT annot);
bool Page_setAnnotEditTextAlign(PDF_PAGE page, PDF_ANNOT annot, int align);

String^ Page_getAnnotEditText(PDF_PAGE page, PDF_ANNOT annot);
bool Page_setAnnotEditFont(PDF_PAGE page, PDF_ANNOT annot, PDF_DOC_FONT font);
bool Page_setAnnotEditTextW( PDF_PAGE page, PDF_ANNOT annot, const wchar_t *text );
unsigned int Page_getAnnotEditTextColor(PDF_PAGE page, PDF_ANNOT annot);
bool Page_setAnnotEditTextColor(PDF_PAGE page, PDF_ANNOT annot, unsigned int color);
int Page_getAnnotComboItemCount( PDF_PAGE page, PDF_ANNOT annot );
String^ Page_getAnnotComboItem(PDF_PAGE page, PDF_ANNOT annot, int item);
int Page_getAnnotComboItemSel( PDF_PAGE page, PDF_ANNOT annot );
bool Page_setAnnotComboItem( PDF_PAGE page, PDF_ANNOT annot, int item );
int Page_getAnnotListItemCount( PDF_PAGE page, PDF_ANNOT annot );
String^ Page_getAnnotListItem(PDF_PAGE page, PDF_ANNOT annot, int item);
int Page_getAnnotListSels( PDF_PAGE page, PDF_ANNOT annot, int *sels, int sels_max );
bool Page_setAnnotListSels( PDF_PAGE page, PDF_ANNOT annot, const int *sels, int sels_cnt );
int Page_getAnnotCheckStatus( PDF_PAGE page, PDF_ANNOT annot );
bool Page_setAnnotCheckValue( PDF_PAGE page, PDF_ANNOT annot, bool check );
bool Page_setAnnotRadio( PDF_PAGE page, PDF_ANNOT annot );
bool Page_getAnnotReset( PDF_PAGE page, PDF_ANNOT annot );
bool Page_setAnnotReset( PDF_PAGE page, PDF_ANNOT annot );
String ^Page_getAnnotSubmitTarget( PDF_PAGE page, PDF_ANNOT annot );
bool Page_getAnnotSubmitParaW( PDF_PAGE page, PDF_ANNOT annot, wchar_t *para, int len );

bool Page_moveAnnot( PDF_PAGE page_src, PDF_PAGE page_dst, PDF_ANNOT annot, const PDF_RECT *rect );
bool Page_copyAnnot( PDF_PAGE page, PDF_ANNOT annot, const PDF_RECT *rect );
bool Page_removeAnnot( PDF_PAGE page, PDF_ANNOT annot );
int Page_exportAnnot(PDF_PAGE page, PDF_ANNOT annot, unsigned char *data, int data_len);
bool Page_importAnnot(PDF_PAGE page, const PDF_RECT *rect, const unsigned char *data, int data_len);
PDF_OBJ_REF Page_getAnnotRef(PDF_PAGE page, PDF_ANNOT annot);
bool Page_addAnnot(PDF_PAGE page, PDF_OBJ_REF ref, int index);
bool Page_addAnnotPopup(PDF_PAGE page, PDF_ANNOT parent, const PDF_RECT *rect, bool open);
bool Page_addAnnotMarkup2(PDF_PAGE page, int ci1, int ci2, int color, int type);
bool Page_addAnnotMarkup( PDF_PAGE page, PDF_MATRIX matrix, const PDF_RECT *rects, int rects_cnt, int color, int type );
bool Page_addAnnotGoto( PDF_PAGE page, PDF_MATRIX matrix, const PDF_RECT *rect, int pageno, float top );
bool Page_addAnnotGoto2( PDF_PAGE page, const PDF_RECT *rect, int pageno, float top );
bool Page_addAnnotUri( PDF_PAGE page, PDF_MATRIX matrix, const PDF_RECT *rect, const char *uri );
bool Page_addAnnotURI2( PDF_PAGE page, const PDF_RECT *rect, const char *uri );
bool Page_addAnnotInk( PDF_PAGE page, PDF_MATRIX matrix, PDF_INK hand, float orgx, float orgy );
bool Page_addAnnotInk2( PDF_PAGE page, PDF_INK hand );
bool Page_addAnnotPolygon(PDF_PAGE page, PDF_PATH hand, int color, int fill_color, float width);
bool Page_addAnnotPolyline(PDF_PAGE page, PDF_PATH hand, int style1, int style2, int color, int fill_color, float width);
bool Page_addAnnotLine( PDF_PAGE page, PDF_MATRIX matrix, const PDF_POINT *pt1, const PDF_POINT *pt2, int style1, int style2, float width, int color, int icolor );
bool Page_addAnnotLine2( PDF_PAGE page, const PDF_POINT *pt1, const PDF_POINT *pt2, int style1, int style2, float width, int color, int icolor );
bool Page_addAnnotRect( PDF_PAGE page, PDF_MATRIX matrix, const PDF_RECT *rect, float width, int color, int icolor );
bool Page_addAnnotRect2( PDF_PAGE page, const PDF_RECT *rect, float width, int color, int icolor );
bool Page_addAnnotEllipse( PDF_PAGE page, PDF_MATRIX matrix, const PDF_RECT *rect, float width, int color, int icolor );
bool Page_addAnnotEllipse2( PDF_PAGE page, const PDF_RECT *rect, float width, int color, int icolor );
bool Page_addAnnotText( PDF_PAGE page, PDF_MATRIX matrix, float x, float y );
bool Page_addAnnotText2( PDF_PAGE page, float x, float y );
bool Page_addAnnotEditbox( PDF_PAGE page, PDF_MATRIX matrix, const PDF_RECT *rect, int line_clr, float line_w, int fill_clr, float tsize, int text_clr );
bool Page_addAnnotEditbox2( PDF_PAGE page, const PDF_RECT *rect, int line_clr, float line_w, int fill_clr, float tsize, int text_clr );
bool Page_addAnnotBitmap( PDF_PAGE page, PDF_MATRIX matrix, PDF_DOC_IMAGE img, const PDF_RECT *rect );
bool Page_addAnnotBitmap2( PDF_PAGE page, PDF_DOC_IMAGE img, const PDF_RECT *rect );
bool Page_addAnnotAttachment( PDF_PAGE page, const char *path, int icon, const PDF_RECT *rect );
bool Page_addAnnotRichMedia(PDF_PAGE page, String ^path_player, String ^path_content, int type, PDF_DOC_IMAGE dimage, const PDF_RECT *rect);


PDF_PATH Path_create();
void Path_moveTo( PDF_PATH path, float x, float y);
void Path_lineTo( PDF_PATH path, float x, float y);
void Path_curveTo( PDF_PATH path, float x1, float y1, float x2, float y2, float x3, float y3 );
void Path_closePath( PDF_PATH path );
void Path_destroy( PDF_PATH path );
int Path_getNodeCount( PDF_PATH path );
int Path_getNode( PDF_PATH path, int index, PDF_POINT *pt );

PDF_PAGECONTENT PageContent_create();
void PageContent_gsSave( PDF_PAGECONTENT content );
void PageContent_gsRestore( PDF_PAGECONTENT content );
void PageContent_gsSet( PDF_PAGECONTENT content, PDF_PAGE_GSTATE gs );
void PageContent_gsSetMatrix( PDF_PAGECONTENT content, PDF_MATRIX mat );
void PageContent_textBegin( PDF_PAGECONTENT content );
void PageContent_textEnd( PDF_PAGECONTENT content );
void PageContent_drawImage( PDF_PAGECONTENT content, PDF_PAGE_IMAGE img );
void PageContent_drawForm(PDF_PAGECONTENT content, PDF_PAGE_FORM form);
PDF_POINT PageContent_textGetSizeW(PDF_PAGECONTENT content, PDF_PAGE_FONT font, const wchar_t *text, float width, float height, float char_space, float word_space);
void PageContent_drawTextW( PDF_PAGECONTENT content, const wchar_t *text );
int PageContent_drawText2W(PDF_PAGECONTENT content, const wchar_t* text, int align, float width);
int PageContent_drawText3W(PDF_PAGECONTENT content, const wchar_t* text, int align, float width, int max_lines);
void PageContent_strokePath( PDF_PAGECONTENT content, PDF_PATH path );
void PageContent_fillPath( PDF_PAGECONTENT content, PDF_PATH path, bool winding );
void PageContent_clipPath( PDF_PAGECONTENT content, PDF_PATH path, bool winding );
void PageContent_setFillColor( PDF_PAGECONTENT content, int color );
void PageContent_setStrokeColor( PDF_PAGECONTENT content, int color );
void PageContent_setStrokeCap( PDF_PAGECONTENT content, int cap );
void PageContent_setStrokeJoin( PDF_PAGECONTENT content, int join );
void PageContent_setStrokeWidth( PDF_PAGECONTENT content, float w );
void PageContent_setStrokeMiter( PDF_PAGECONTENT content, float miter );
void PageContent_setStrokeDash(PDF_PAGECONTENT content, const float* dash, int dash_cnt, float phase);
void PageContent_textSetCharSpace( PDF_PAGECONTENT content, float space );
void PageContent_textSetWordSpace( PDF_PAGECONTENT content, float space );
void PageContent_textSetLeading( PDF_PAGECONTENT content, float leading );
void PageContent_textSetRise( PDF_PAGECONTENT content, float rise );
void PageContent_textSetHScale( PDF_PAGECONTENT content, int scale );
void PageContent_textNextLine( PDF_PAGECONTENT content );
void PageContent_textMove( PDF_PAGECONTENT content, float x, float y );
void PageContent_textSetFont( PDF_PAGECONTENT content, PDF_PAGE_FONT font, float size );
void PageContent_textSetRenderMode( PDF_PAGECONTENT content, int mode );
void PageContent_destroy( PDF_PAGECONTENT content );
PDF_PAGE_FONT Page_addResFont( PDF_PAGE page, PDF_DOC_FONT font );
PDF_PAGE_IMAGE Page_addResImage( PDF_PAGE page, PDF_DOC_IMAGE image );
PDF_PAGE_GSTATE Page_addResGState( PDF_PAGE page, PDF_DOC_GSTATE gstate );
PDF_PAGE_FORM Page_addResForm(PDF_PAGE page, PDF_DOC_FORM form);
bool Page_flate(PDF_PAGE page);
bool Page_flateAnnot(PDF_PAGE page, PDF_ANNOT annot);
bool Page_addContent( PDF_PAGE page, PDF_PAGECONTENT content, bool flush );


int Obj_dictGetItemCount(PDF_OBJ hand);
const char *Obj_dictGetItemName(PDF_OBJ hand, int index);
PDF_OBJ Obj_dictGetItemByIndex(PDF_OBJ hand, int index);
PDF_OBJ Obj_dictGetItemByName(PDF_OBJ hand, const char *name);
void Obj_dictSetItem(PDF_OBJ hand, const char *name);
void Obj_dictRemoveItem(PDF_OBJ hand, const char *name);
int Obj_arrayGetItemCount(PDF_OBJ hand);
PDF_OBJ Obj_arrayGetItem(PDF_OBJ hand, int index);
void Obj_arrayAppendItem(PDF_OBJ hand);
void Obj_arrayInsertItem(PDF_OBJ hand, int index);
void Obj_arrayRemoveItem(PDF_OBJ hand, int index);
void Obj_arrayClear(PDF_OBJ hand);
bool Obj_getBoolean(PDF_OBJ hand);
void Obj_setBoolean(PDF_OBJ hand, bool v);
int Obj_getInt(PDF_OBJ hand);
void Obj_setInt(PDF_OBJ hand, int v);
float Obj_getReal(PDF_OBJ hand);
void Obj_setReal(PDF_OBJ hand, float v);
const char *Obj_getName(PDF_OBJ hand);
void Obj_setName(PDF_OBJ hand, const char *v);
const char *Obj_getAsciiString(PDF_OBJ hand);
String ^Obj_getTextString(PDF_OBJ hand);
unsigned char *Obj_getHexString(PDF_OBJ hand, int *len);
void Obj_setAsciiString(PDF_OBJ hand, const char *v);
void Obj_setTextString(PDF_OBJ hand, const wchar_t *v);
void Obj_setHexString(PDF_OBJ hand, unsigned char *v, int len);
PDF_OBJ_REF Obj_getReference(PDF_OBJ hand);
void Obj_setReference(PDF_OBJ hand, PDF_OBJ_REF v);
int Obj_getType(PDF_OBJ hand);
PDF_OBJ Document_advGetObj(PDF_DOC doc, PDF_OBJ_REF ref);
PDF_OBJ_REF Document_advNewIndirectObj(PDF_DOC doc);
PDF_OBJ_REF Document_advNewIndirectObjWithData(PDF_DOC doc, PDF_OBJ obj_hand);
PDF_OBJ_REF Document_advGetRef(PDF_DOC doc);
void Document_advReload(PDF_DOC doc);
PDF_OBJ_REF Document_advNewFlateStream(PDF_DOC doc, const unsigned char *source, int len);
PDF_OBJ_REF Document_advNewRawStream(PDF_DOC doc, const unsigned char *source, int len);
PDF_OBJ_REF Page_advGetAnnotRef(PDF_PAGE page, PDF_ANNOT annot);
PDF_OBJ_REF Page_advGetRef(PDF_PAGE page);
void Page_advReloadAnnot(PDF_PAGE page, PDF_ANNOT annot);
void Page_advReload(PDF_PAGE page);

#ifdef __cplusplus
}
#endif

#endif
