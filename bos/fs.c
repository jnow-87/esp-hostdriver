/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <bos/fs.h>
#include <sys/list.h>
#include <string.h>


/* macros */
#define FMODE_DEFAULT	(O_NONBLOCK)


/* static variables */
static fs_t *fs_lst = 0x0;
static fs_filed_t *fd_lst = 0x0;


/* global functions */
int fs_register(fs_ops_t *ops){
	int id;
	fs_t *fs;


	id = 0;


	if(!list_empty(fs_lst))
		id = list_last(fs_lst)->id + 1;

	if(id < 0)
		goto err;

	if(ops == 0x0 || ops->open == 0x0 || ops->close == 0x0)
		goto err;

	fs = malloc(sizeof(fs_t));

	if(fs == 0x0)
		goto err;

	fs->id = id;
	fs->ops = *ops;

	list_add_tail(fs_lst, fs);


	return fs->id;


err:
	return -1;
}

fs_filed_t *fs_fd_alloc(fs_node_t *node, f_mode_t mode, f_mode_t mode_mask){
	static int id = 0;
	fs_filed_t *fd;


	/* acquire descriptor id */
	id++;

	/* allocate file descriptor */
	fd = malloc(sizeof(fs_filed_t));

	if(fd == 0x0)
		goto err_0;

	fd->id = id;
	fd->node = node;
	fd->fp = 0;
	fd->mode = (mode & ~mode_mask) | (FMODE_DEFAULT & mode_mask);
	fd->mode_mask = mode_mask;

	list_add_tail(fd_lst, fd);
	node->ref_cnt++;

	return fd;


err_0:
	return 0x0;
}

void fs_fd_free(fs_filed_t *fd){
	list_rm(fd_lst, fd);
	fd->node->ref_cnt--;
	free(fd);
}

fs_filed_t *fs_fd_acquire(int id){
	fs_filed_t *fd;


	fd = list_find(fd_lst, id, id);

	return fd;
}

void fs_fd_release(fs_filed_t *fd){
}

fs_node_t *fs_node_create(fs_node_t *parent, char const *name, size_t name_len, file_type_t type, void *data, int fs_id){
	fs_t *fs;
	fs_node_t *node;


	if(name_len + 1 > 32)
		goto err_0;

	/* identify file system */
	list_for_each(fs_lst, fs){
		if(fs->id == fs_id)
			break;
	}

	if(fs == 0x0)
		goto err_0;

	/* allocate node */
	node = malloc(sizeof(fs_node_t) + name_len + 1);

	if(node == 0x0)
		goto err_0;

	/* init node attributes */
	node->fs_id = fs_id;
	node->ops = &fs->ops;
	node->ref_cnt = 0;
	node->type = type;

	strncpy(node->name, name, name_len);
	node->name[name_len] = 0;

	node->parent = parent;
	node->childs = 0x0;
	node->data = data;

	/* add node to file system */
	if(parent)
		list_add_tail(parent->childs, node);

	/* add '.' and '..' nodes for directories */
	if(type == FT_DIR){
		if(fs_node_create(node, ".", 1, FT_LNK, node, fs_id) == 0x0)
			goto err_1;

		if(parent){
			if(fs_node_create(node, "..", 2, FT_LNK, parent, parent->fs_id) == 0x0)
				goto err_1;
		}
	}

	return node;


err_1:
	fs_node_destroy(node);

err_0:
	return 0x0;
}

int fs_node_destroy(fs_node_t *node){
	fs_node_t *child;


	if(node->ref_cnt > 0)
		goto err;

	list_for_each(node->childs, child){
		if(fs_node_destroy(child) != 0)
			goto err;
	}

	list_rm(node->parent->childs, node);

	free(node);

	return 0;


err:
	return -1;
}
