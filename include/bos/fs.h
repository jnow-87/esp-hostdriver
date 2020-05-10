#ifndef FS_H
#define FS_H


#include <bos/fcntl.h>
#include <bos/stat.h>


/* incomplete types */
struct fs_filed_t;
struct fs_node_t;


/* types */
typedef struct process_t{
} process_t;

// file system types
typedef struct{
	/**
	 * \brief	Create file descriptor for the file specified through start and path,
	 * 			starting the search at file system node start. Also perform any
	 * 			allocations if required for individual file systems. The resulting
	 * 			descriptor is finally added to this_p's list.
	 *
	 * \param	start	File system node to start search from
	 * \param	path	Path to the target file
	 * \param	mode	Mode to consider when opening the file. Individual behaviour
	 * 					depends on the concrete file system implementation.
	 * \param	this_p	process that the file descriptor is allocated to
	 *
	 * \return	Id of the file descriptor allocated or a value smaller than 0 if an
	 * 			error occurred. In the latter case errno is set appropriately.
	 *
	 * \post	If the target file has been found a file descriptor is allocated
	 * 			using fs_fd_alloc().
	 */
	int (*open)(struct fs_node_t *start, char const *path, f_mode_t mode, struct process_t *this_p);

	/**
	 * \brief	Release the given file descriptor and revert file system dependent
	 * 			allocations performed through the respective open() callback. Also
	 * 			remove the descriptor from this_p's list.
	 *
	 * \param	fd		pointer to target file descriptor
	 * \param	this_p	process that the file descriptor is allocated to
	 *
	 * \return	E_OK on success and a value smaller than 0 if an error occurred. In
	 * 			the latter case errno is set appropriately.
	 *
	 * \post	If no error occurred the target file descriptor is released using
	 * 			fs_fd_free().
	 */
	int (*close)(struct fs_filed_t *fd, struct process_t *this_p);

	/**
	 * \brief	Copy utmost n bytes from the file associated with fd to buffer buf.
	 *
	 * \param	fd		target file descriptor
	 * \param	buf		buffer to copy to (kernel memory)
	 * \param	n		maximum number to copy to buf
	 *
	 * \return	Number of copied bytes. If an error occurred 0 is returned and error
	 * 			is set appropriately.
	 */
	size_t (*read)(struct fs_filed_t *fd, void *buf, size_t n);

	/**
	 * \brief	Copy utmost n bytes from buffer buf to the file associated with fd.
	 *
	 * \param	fd		target file descriptor
	 * \param	buf		buffer to copy from (kernel memory)
	 * \param	n		maximum number to copy to buf
	 *
	 * \return	Number of copied bytes. If an error occurred 0 is returned and error
	 * 			is set appropriately.
	 */
	size_t (*write)(struct fs_filed_t *fd, void *buf, size_t n);

	/**
	 * \brief	Perform the given device manipulation defined by request on the file
	 * 			associated with fd.
	 *
	 * \param	fd			target file descriptor
	 * \param	request		Operation to perform. Behaviour depends on the individual
	 * 						file system implementation.
	 * \param	data		pointer to additional data that might be required to
	 * 						perform the request (kernel memory)
	 *
	 * \return	E_OK on succes. If an error occured a value smaller than 0 is
	 * 			returned and errno is set appropriately.
	 */
	int (*ioctl)(struct fs_filed_t *fd, int request, void *data);

	/**
	 * \brief	Perform the requested command on the given file descriptor fd.
	 *
	 * \param	fd			target file descriptor
	 * \param	request		Command to perform. Behaviour depends on the individual
	 * 						file system implementation.
	 * \param	data		pointer to additional data that might be required to
	 * 						perform the request (kernel memory)
	 *
	 * \return	E_OK on succes. If an error occured a value smaller than 0 is
	 * 			returned and errno is set appropriately.
	 */
	int (*fcntl)(struct fs_filed_t *fd, int cmd, void *data);

	/**
	 * \brief	Remove the node specified through start and path from the file
	 * 			system.
	 *
	 * \param	start	root node to start the search for the target node
	 * \param	path	path to the target node
	 *
	 * \return	E_OK if the target node has been removed. On error a value smaller
	 * 			than 0 is returned and errno is set appropriately.
	 */
	int (*node_rm)(struct fs_node_t *start, char const *path);

	/**
	 * \brief	Return the node specified through start and path.
	 *
	 * \param	start	root node to start the search for the target node
	 * \param	path	path to the target node
	 *
	 * \return	pointer to the target node on success. On error 0x0 is returned and
	 * 			errnor is set appropriately.
	 */
	struct fs_node_t *(*node_find)(struct fs_node_t *start, char const *path);
} fs_ops_t;

typedef struct fs_t{
	struct fs_t *prev,
				*next;

	int id;
	fs_ops_t ops;
} fs_t;

// file system node types
typedef struct fs_node_t{
	struct fs_node_t *prev,
					 *next;

	int fs_id;
	void *ops;

	void *data;
	unsigned int ref_cnt;
	file_type_t type;

	struct fs_node_t *childs,
					 *parent;

	char name[];
} fs_node_t;

// file descriptor types
typedef struct fs_filed_t{
	struct fs_filed_t *prev,
					  *next;

	int id;

	size_t fp;
	fs_node_t *node;
	f_mode_t mode,
			 mode_mask;
} fs_filed_t;


/* prototypes */
// file system operations
int fs_register(fs_ops_t *ops);

// file operations
fs_filed_t *fs_fd_alloc(fs_node_t *node, f_mode_t mode, f_mode_t mode_mask);
void fs_fd_free(fs_filed_t *fd);

fs_filed_t *fs_fd_acquire(int id);
void fs_fd_release(fs_filed_t *fd);

fs_node_t *fs_node_create(fs_node_t *parent, char const *name, size_t name_len, file_type_t type, void *data, int fs_id);
int fs_node_destroy(fs_node_t *node);


#endif // FS_H
