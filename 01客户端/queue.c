#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "queue.h"

/*
创建队列 
	即创建一个空的头节点
*/
Queue* create_queue(void)
{
	Queue* q = (Queue*)malloc(sizeof(Queue));
	if(q == NULL)
		{
			perror("malloc Queue error");
			return  NULL;
		}
	q->head = q->tail = NULL;
	q->num = 0;
	return q;
}

/*
入队
	即采用尾插法插入节点 
*/

int Enqueue(Queue* q,Element *n)
{
	Node *p = (Node*)malloc(sizeof(Node));
	if(p == NULL)
		{
			perror("malloc node error");
			return -1;
		}
	
	memcpy(p->data,n,640*480*2);
	printf("the image zise = %d\n",strlen(p->data));
	p->next = NULL;
	
	if(q->head == NULL)
		{
			q->head = q->tail = p;
			q->num++;
		}
	else 
		{
			q->tail->next = p;
			q->tail = p;
			q->num++;
		}
	return 0;
}
/*
出队 
	判断不为空则删除头 注意头结点的数据依然要返回 
*/
int del_queue(Queue* q,Element *n)
{
	if(!Queue_IS_Empty(q))
	{
		memcpy(n,q->head->data,640*480*2);
		Node *r = q->head;
		if(r ==q->tail)
			q->tail = NULL;
		q->head = r->next;
		r->next = NULL;
		free(r);
		r = NULL;
		q->num--;
		return 0;
	}
	return -1;
}
/*
判断队是否为空 
	即 判断head == NULL 即可 
*/

int Queue_IS_Empty(Queue* q)
{
	if(q->head == NULL)
	{
		return 1;
	}
	return 0;
}
/*
清空队 
	即无条件所有出队 
	*/
int clear_queue(Queue* q)
	{
		Element data[640*480*2];
		while(!del_queue(q,data));
		return 0;
	}
/*
销毁队 
	清空后 free 
	*/
Queue* destory_Queue(Queue* q)
	{
		clear_queue(q);
		free(q);
		q = NULL;
		return q;
	}
/*
获取队首 
	return head->data 
获取队长 
	return queue->num 
*/