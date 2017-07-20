/****************************************************************************
* ����: �����ʱ��
*       �ɼ򻯶�ʱ����Ĵ����д
* ������<kerndev@foxmail.com>
* 2017.7.20
****************************************************************************/
#include "kernel.h"
#include "list.h"
#include "timer.h"

struct timer
{
	struct timer* prev;
	struct timer* next;
	int id;
	int counter;
	int reload;
	void(*callback)(int);
};

struct timer_list
{
	struct timer* head;
	struct timer* tail;
	kmutex_t mutex;
	kevent_t event;
};

static struct timer_list list;

//�����㣬ʱ���Ѿ�����tick��ô����!!!!
//����Һúü��һ�¶�ʱ��, ��ʱ�䵽�˵Ķ����Ҵ�����!!!!
//Ȼ���ٸ����Ҿ�����һ����ʱ���������ж��!!!!
static int timer_timetick(int tick)
{
	int min;
	struct timer* node;
	min = 1000;
	kmutex_lock(list.mutex);
	for(node=list.head; node!=NULL; node=node->next)
	{
		if(node->counter > 0)
		{
			node->counter -= tick;
			if(node->counter <= 0)
			{
				node->counter = node->reload;
				node->callback(node->id);
			}
		}
		else
		{
			node->counter = node->reload;
		}
		if(node->counter < min)
		{
			min = node->counter;
		}
	}
	kmutex_unlock(list.mutex);
	return min;
}

//��ʱ���߳�
//���ȴ���һ����ʱ��������ô�õ�ʱ�䣬��Ҫȥ���һ��!
static void timer_thread_entry(void* arg)
{
	int tick;
	int timeout;
	uint32_t tick;
	timeout = 0;
	while(1)
	{
		tick = kernel_tick();
		kevent_timedwait(list.event, timeout);
		tick = kernel_tick() - tick;
		timeout = timer_timetick(tick);
	}
}

//������ʱ��
timer_t timer_create(int id, int elapse, void(*callback)(int))
{
	struct timer* node;
	node = kmem_alloc(sizeof(struct timer));
	if(node != NULL)
	{
		node->id = id;
		node->counter= 0;
		node->reload = elapse;
		node->callback = callback;
		kmutex_lock(list.mutex);
		list_append(&list, node);
		kmutex_unlock(list.mutex);
		kevent_post(list.event);
	}
	return node;
}

//��ʼ����ʱ��ģ��
//���ö�ʱ���̵߳Ķ�ջ��С���߳����ȼ�
void timer_init(uint32_t stk_size, int prio)
{
	kthread_t thread;
	list_init(&list);
	thread = kthread_craete(timer_thread_entry, 0, stk_size);
	kthread_setprio(thread, prio);
}
