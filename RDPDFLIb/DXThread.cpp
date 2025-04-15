#include "pch.h"
#include "DXThread.h"
#include "DXBlock.h"
#include "DXFinder.h"
using namespace RDPDFLib::view;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Concurrency;
using namespace Platform;

bool DXThread::start()
{
	if (m_run) return true;
	queue_event.reset();
	m_eve.reset();
	m_run = true;
	ThreadPool::RunAsync(ref new WorkItemHandler([this](IAsyncAction ^act)
	{
		QUEUE_NODE node;
		DXBlock *blk;
		while (true)
		{
			get_msg(node);
			if (node.mid == 0xFFFFFFFF) break;
			switch (node.mid)
			{
			case 1:
				blk = (DXBlock *)node.para1;
				blk->bk_render();
				m_disp->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, blk](){
					m_callback->onVCacheRendered(blk);
				}));
				break;
			case 2:
				((DXBlock *)node.para1)->bk_destroy();
				delete (DXBlock *)node.para1;
				break;
			case 3:
				int ret = ((DXFinder *)node.para1)->find();
				m_disp->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, ret]() {
					m_callback->onVFound(ret);
				}));
				break;
			}
			node.para1 = NULL;
			node.para2 = NULL;
		}
		m_eve.notify();
	}), WorkItemPriority::Normal, WorkItemOptions::TimeSliced);
	return true;
}

void DXThread::render_start(DXBlock *blk)
{
	if (blk && blk->dx_start())
	{
		post_msg(1, blk, NULL);
	}
}

bool DXThread::render_end(DXBlock *blk)
{
	if (!blk) return false;
	if (blk->dx_end())
	{
		post_msg(2, blk, NULL);
		return true;
	}
	else return false;
}
