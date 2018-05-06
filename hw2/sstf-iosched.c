/*
 * elevator C-LOOK
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

struct sstf_data {
	struct list_head queue;
	int head;
};

static void sstf_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
}

static int sstf_dispatch(struct request_queue *q, int force)
{
	struct sstf_data *sd = q->elevator->elevator_data;
	int sector = 0;
	char operation = 'E';

	if (!list_empty(&sd->queue)) {
		struct request *rq;
		rq = list_entry(sd->queue.next, struct request, queuelist);
		list_del_init(&rq->queuelist);
		elv_dispatch_sort(q, rq);
		//debug printout
		if (rq_data_dir(rq) == 0){
			operation = 'R';
		}
		else {
			operation = 'W';
		}
		sector = blk_rq_pos(rq);
		printk("IO dispatch request: %c at sector %d\n",operation,sector);
		sd->head = sector;
		
		return 1;
	}
	return 0;
}

static void sstf_add_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *sd = q->elevator->elevator_data;
	struct list_head *rq_pos;
	int sector = 0;
	
	//if the list is empty, we just add it
	if(list_empty(&sd->queue)){
		printk("Add request: Queue was empty, item added.\n");
		list_add_tail(&rq->queuelist, &sd->queue);
	}
	else {
		list_for_each(rq_pos, &sd->queue){
			struct request *s_rq = list_entry(rq_pos, struct request, queuelist);
			if (blk_rq_pos(rq) >= sd->head){
				if(sd->head > blk_rq_pos(s_rq)){
					break;
				}
				else if (blk_rq_pos(s_rq) > blk_rq_pos(rq)){
					break;
				}
			}
			else {
				if ((sd->head > blk_rq_pos(s_rq)) && (blk_rq_pos(s_rq) > blk_rq_pos(rq))){
					break;
				}
			}
		}
		
		sector = blk_rq_pos(rq);
		printk("Add request: add item at sector %d\n",sector);
		list_add_tail(&rq->queuelist, rq_pos);
	}
}

static struct request *
sstf_former_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *sd = q->elevator->elevator_data;

	if (rq->queuelist.prev == &sd->queue)
		return NULL;
	return list_entry(rq->queuelist.prev, struct request, queuelist);
}

static struct request *
sstf_latter_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *sd = q->elevator->elevator_data;

	if (rq->queuelist.next == &sd->queue)
		return NULL;
	return list_entry(rq->queuelist.next, struct request, queuelist);
}

static int sstf_init_queue(struct request_queue *q, struct elevator_type *e)
{
	struct sstf_data *sd;
	struct elevator_queue *eq;

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	sd = kmalloc_node(sizeof(*sd), GFP_KERNEL, q->node);
	if (!sd) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = sd;

	INIT_LIST_HEAD(&sd->queue);

	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);
	printk("Queue successfully initialized\n");
	return 0;
}

static void sstf_exit_queue(struct elevator_queue *e)
{
	struct sstf_data *sd = e->elevator_data;

	BUG_ON(!list_empty(&sd->queue));
	kfree(sd);
}

static struct elevator_type elevator_sstf = {
	.ops = {
		.elevator_merge_req_fn		= sstf_merged_requests,
		.elevator_dispatch_fn		= sstf_dispatch,
		.elevator_add_req_fn		= sstf_add_request,
		.elevator_former_req_fn		= sstf_former_request,
		.elevator_latter_req_fn		= sstf_latter_request,
		.elevator_init_fn		= sstf_init_queue,
		.elevator_exit_fn		= sstf_exit_queue,
	},
	.elevator_name = "sstf",
	.elevator_owner = THIS_MODULE,
};

static int __init sstf_init(void)
{
	return elv_register(&elevator_sstf);
}

static void __exit sstf_exit(void)
{
	elv_unregister(&elevator_sstf);
}

module_init(sstf_init);
module_exit(sstf_exit);


MODULE_AUTHOR("Group 16");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SSTF I/O scheduler");
