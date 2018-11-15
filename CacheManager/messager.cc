#include "messager.h"

GMessager* GMessager::messager_;
std::list<Observer*> GMessager::handlers_;

GMessager * GMessager::GetInstance()
{
	// TODO: no thread safe
	if (!messager_)
	{
		return new GMessager();
	}
	return messager_;
}

GMessager::GMessager()
{
}


GMessager::~GMessager()
{
}

void GMessager::PostMsg(const Message& msg)
{
	// TODO: ¼ÓËø
	for (auto& it : handlers_)
	{
		it->Process(msg);
	}
}

void GMessager::Subcrible(Observer * handler)
{
	handlers_.push_back(handler);
}

void GMessager::Unsubcrible(Observer * handler)
{
	handlers_.remove(handler);
}

void GMessager::Process(const Message& msg)
{
	//while (GetMsg(msg))
	//{
	//	handler_->
	//}
}

//void GMessager::GetMsg(Message & msg)
//{
//}
