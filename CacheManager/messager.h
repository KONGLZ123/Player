#pragma once
#include <vector>
#include <list>
#include <map>

enum MsgType
{
	MediaMsg = 0,
	PreloadMsg = 1,
};

enum MsgId
{
	PreloadProgram = 0,
	SwitchLastProgram,
	SwitchNextProgram,
	SwitchRightProgram,
	PlayProgram,
	StopProgram,
    NotifyParser,
    HttpError,
    HttpComplete,
};

struct Message
{
	int msg_type;
	int msg_id;
	std::vector<std::string> params;
};

class Observer
{
public:
	virtual void Process(const Message& msg) = 0;
};

class GMessager
{
public:
	static GMessager* GetInstance();
	void PostMsg(const Message& msg);

	void Subcrible(Observer* handler);
	void Unsubcrible(Observer* handler);

private:
	GMessager();
	~GMessager();

	virtual void Process(const Message& msg);
	//void GetMsg(Message& msg);

private:
	static GMessager* messager_;
	//std::list<Message> msg_list_;
	static std::list<Observer*> handlers_;
};



