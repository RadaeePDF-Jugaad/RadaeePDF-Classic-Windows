#include "pch.h"
#include "RDVBlk.h"
#include "RDVFinder.h"
#include "RDVThread.h"
using namespace RDPDFLib::view;
int CRDVBlk::m_cell_size = 0;
PDFBmp^ CRDVBlk::m_def_bmp = nullptr;


void CRDVThread::render_start(CRDVBlk* blk)
{
	if (blk && blk->ui_start())
		post_msg(0, blk);
}

bool CRDVThread::render_end(IVCallback^ canvas, CRDVBlk* blk)
{
	if (!blk) return false;
	if (blk->ui_end(canvas))
	{
		post_msg(1, blk);
		return true;
	}
	else return false;
}

CRDVBlk* CRDVThread::render_end2(IVCallback^ canvas, CRDVBlk* blk)
{
	if (!blk) return blk;
	if (blk->ui_end(canvas))
	{
		CRDVBlk *ret = new CRDVBlk(blk);
		post_msg(1, blk);
		return ret;
	}
	else return blk;
}


DWORD WINAPI CRDVThread::ProcWork(void* para)
{
	CRDVThread* thiz = (CRDVThread*)para;
	QUEUE_NODE node;
	int count = 0;
	while (thiz->get_msg(node))
	{
		switch (node.mid)
		{
		case 0:
			((CRDVBlk*)node.para1)->bk_render();
			count++;
			if(thiz->m_disp)
				thiz->m_disp->RunAsync(CoreDispatcherPriority::Normal, thiz->m_render);
			break;
		case 1:
			//((CRDVBlk*)node.para1)->bk_destroy();
			count--;
			delete (CRDVBlk*)node.para1;
			break;
		case 2:
			int ret = ((CRDVFinder*)node.para1)->find();
			if (thiz->m_disp)
				thiz->m_disp->RunAsync(CoreDispatcherPriority::Normal, thiz->m_finder);
			break;
		}
		node.para1 = NULL;
	}
	if (thiz->m_disp)
		thiz->m_disp->RunAsync(CoreDispatcherPriority::Normal, thiz->m_destroy);
	return 0;
}
