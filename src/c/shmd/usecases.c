#include "shm_object.h"
#include "file_xfer.h"
#include "file_xfer_client.h"
#include "file_xfer_server.h"

void fxr_server_main(const char *shmname)
{
	shm_object *shmobj = shm_object_create(shmname, FILE_XFER);
	fxr_create(shmobj->obj.fxr);
	file_xfer_server(shmobj->obj.fxr);
	fxr_destroy(shmobj->obj.fxr);
	shm_object_destroy(shmobj);
}

void fxr_client_main(const char *src_host, const char *src_file, const char *dest_file)
{
	shm_object *shmobj = shm_object_attach(src_host, FILE_XFER);
	file_xfer_client(shmobj->obj.fxr, src_file, dest_file);
	shm_object_destroy(shmobj);
}
