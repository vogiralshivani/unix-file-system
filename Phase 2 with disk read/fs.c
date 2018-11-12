#include <stdio.h>
#include <stdlib.h>
#include <fuse.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

#include "fs.h"

//int inode_bitmap[no_of_blocks];
//int data_bitmap[no_of_blocks];

//Inode inodeblocks[no_of_blocks];

char* metafilename = "fsmeta";
char *datafilename = "fsdata";

meta_node* init_meta_node(Inode* I)
{
    int i;
    meta_node *temp = (meta_node*)malloc(sizeof(meta_node));
    temp->pathlength = strlen(I->path);
    temp->path = (char*)malloc(strlen(I->path)*sizeof(char));
    strcpy(temp->path, I->path);
    temp->namelength = strlen(I->name);
    temp->name = (char*)malloc(strlen(I->name)*sizeof(char));
    strcpy(temp->name, I->name);
	temp->parent_namelength = strlen(I->parent_name);
	temp->parent_name = (char*)malloc(strlen(I->parent_name)*sizeof(char));
	strcpy(temp->parent_name, I->parent_name);
    temp->type = I->type;
    temp->no_of_children = I->no_of_children;
    temp->size = I->size;
    temp->inode_num =I->inode_num;
    if(I->inode_num != 0)
	{
        temp->parent_inode = I->parent->inode_num;
		temp->parent_name = I->parent->name;
	}
    else
	{
        temp->parent_inode = -1;
		temp->parent_name = NULL;
	}
    if(I->no_of_children > 0)
    {
        temp->child_inode = (int*)malloc(temp->no_of_children*sizeof(int));
        for(i=0; i<temp->no_of_children; i++)
            temp->child_inode[i] = I->child_inode[i];
    }
    else 
        temp->child_inode = NULL;

    temp->file.size = I->file.size;
    printf("I FILE SIZE: %d\n", I->file.size);
    if(I->file.data != NULL)
    {
        temp->file.data = (char*)malloc(strlen(I->file.data)*sizeof(char));
        strcpy(temp->file.data, I->file.data);
    }
    return temp;
}
void write_to_disk_wrapper()
{
    printf("here in disk write_to_disk_wrapper\n"); 
     
    FILE *mfp = fopen(metafilename,"wb");
    FILE *dfp = fopen(datafilename, "wb");
    //int i=0;
    Inode *temp = root;
    meta_node *temp_disk = init_meta_node(root);
    //writing root node
    //inode_bitmap[i] = 1;
    //data_bitmap[i] = 1;
    //inodeblocks[i] = *temp;
    //i++;
    //fwrite(temp_disk,sizeof(temp_disk),1,mfp);
    write_to_disk(temp_disk, mfp, dfp);
    printf("WRITTEN to DISK\n");
    free(temp_disk);
    int j;
    int n=temp->no_of_children;
    for(j=0;j<n;j++)
    {
        printf("in for\n");
        write_to_disk_recurse(temp->children[j],mfp, dfp);
    }
    printf("Closing\n");
    fclose(mfp);
    fclose(dfp);

}

void write_to_disk_recurse(Inode *node, FILE *mfp, FILE *dfp)
{
    printf("in write_to_disk recurse\n");
    //static int i=1;
    //if(node->type==0)
    //{
        //inode_bitmap[*i] = 1;
       
        //inodeblocks[*i] = *node;

        //*(i++);   
    meta_node *temp_disk = init_meta_node(node);
    printf("temp disk set\n");
    write_to_disk(temp_disk, mfp, dfp);
    //fwrite(temp_disk,sizeof(temp_disk),1,mfp);
    printf("WRITTEN TO DISK\n");
    free(temp_disk);
        //printf("i = %d\n",*i );

    //}   
    //else
    //{
        int j=0;
        for(j=0;j<node->no_of_children;j++)
        {
            write_to_disk_recurse(node->children[j], mfp, dfp);
        }
    //}
    return;
}

void write_to_disk(meta_node* disk, FILE *mfp, FILE *dfp)
{
    fwrite(&disk->pathlength, sizeof(disk->pathlength), 1, mfp);
    printf("Pathlen written\n");
    fwrite(disk->path, sizeof(disk->path), 1, mfp);
    printf("Path written. sizeof path: %ld\n", sizeof(disk->path));
    fwrite(&disk->namelength, sizeof(disk->namelength), 1, mfp);
    printf("Namelen written %d\n", disk->namelength);
    fwrite(disk->name, sizeof(disk->name), 1, mfp);
    printf("Name written\n %s", disk->name);
	fwrite(&disk->parent_namelength, sizeof(disk->parent_namelength), 1,mfp);
	printf("Parent Namelength written\n");
	fwrite(disk->parent_name, sizeof(disk->parent_name),1,mfp);
	printf("Parent Name Written. Length is %d\n",sizeof(disk->parent_name));
    fwrite(&disk->type, sizeof(disk->type), 1, mfp);
    printf("Type written\n");
    fwrite(&disk->no_of_children, sizeof(disk->no_of_children), 1, mfp);
    printf("No_children written\n");
    fwrite(&disk->size, sizeof(disk->size), 1, mfp);
    printf("Size written\n");
    fwrite(&disk->inode_num, sizeof(disk->inode_num), 1, mfp);
    printf("Inode_num written\n");
    if(disk->child_inode != NULL)
        fwrite(disk->child_inode, sizeof(disk->child_inode), 1, mfp);
    printf("Child_inode written\n");
    fwrite(&disk->parent_inode, sizeof(disk->parent_inode), 1, mfp);
    printf("Parent written\n");
    fwrite(&disk->file.size, sizeof(disk->file.size), 1, mfp);
    printf("File size written %d\n", disk->file.size);
    
    if(disk->file.size != 0)
    {
        data_node *regfile = (data_node*)malloc(sizeof(data_node));
        regfile->inode_num = disk->inode_num;
        printf("WRITING TO DATA FILE %d\n", regfile->inode_num);
        fwrite(&regfile->inode_num, sizeof(regfile->inode_num), 1, dfp);
        regfile->size = disk->file.size;
        fwrite(&regfile->size, sizeof(regfile->size), 1, dfp);
        regfile->data  = (char*)malloc(regfile->size*sizeof(char));
        strcpy(regfile->data, disk->file.data);
        fwrite(regfile->data, sizeof(regfile->inode_num), 1, dfp);
        free(regfile);
    }
    printf("Filedata written\n");
}
//HELPER FUNCTIONS
/*char *extractPath(char ** copy_path)
{
    printf("Path being extracted\n");
    char *path = (char *)calloc(sizeof(char), 1);
    int p_len = 0;
    char temp;
    char *tempstr;
    temp = **(copy_path);
    while(temp != '\0')
    {    
        if(temp == '/')
        {
            if(strlen(*copy_path) > 1)
            {
                (*copy_path)++;
            }
            break;
        }
        tempstr = (char *)calloc(sizeof(char) , (p_len + 2));
        strcpy(tempstr, path);
        p_len += 1;
        tempstr[p_len - 1] = temp;
        path = (char *)realloc(path, sizeof(char) * (p_len + 2));
        strcpy(path, tempstr);
        (*copy_path)++;
        temp = **(copy_path);
        free(tempstr);
    }
    path = (char *)realloc(path, sizeof(char) * (p_len + 1));
    path[p_len] = '\0';
    return path;
}
char *reverse(char *str, int mode)
{
    printf("Reversing String\n");
    int i;
    int len = strlen(str);
    char *tempstr = (char *)calloc(sizeof(char), (len + 1));
    for(i = 0; i <= len/2; i++){
        tempstr[i] = str[len - 1 -i];
        tempstr[len - i - 1] = str[i];
    }
    if(tempstr[0] == '/' && mode == 1)
    {
        /*if mode is set to 1 then it is a directory
        and return the string without the leading '/'
        */ 
/*        tempstr++;
    }
    return tempstr;
}*/


char *extractDir(char *path){
    printf("EXTRACT DIR CALLED\n");
    /*printf("Extracting Directory Name\n");
    char *dir = (char *)calloc(sizeof(char), 1);
    int d_len = 0;
    char temp;
    char * tempstr;
    *copy_path = reverse(*copy_path, 1);    // change "a/b/c" to "c/b/a" and extract content upto the first '/'
    temp = **(copy_path);
    printf("TEMP: %s\n", *copy_path);
    while(temp != '/'){    
        tempstr = (char *)calloc(sizeof(char), (d_len + 2));
        strcpy(tempstr, dir);
        d_len += 1;
        tempstr[d_len - 1] = temp;
        dir = (char *)realloc(dir, sizeof(char) * (d_len + 1));
        strcpy(dir, tempstr);
        (*copy_path)++;
        temp = **(copy_path);
        free(tempstr);
    }
    if(strlen(*copy_path) > 1){
        (*copy_path)++;                     // remove the leading '/' from "/b/a" after extracting 'c'
    }
    dir = (char *)realloc(dir, sizeof(char) * (d_len + 1));
    dir[d_len] = '\0';
    dir = reverse(dir, 0);        
    *(copy_path) = reverse(*(copy_path), 0);*/
    printf("exname_dir:%s\n", path);
    if(strcmp(path,"/")==0)
        return path;
    char str[strlen(path)];
    strcpy(str,path);
    char* token = strtok(str, "/"),*prev_token = malloc(sizeof(char)*strlen(path)), *token_dir = malloc(sizeof(char)*strlen(path)); 
    strcpy(prev_token, "/");
    while (token != NULL) { 
        strcpy(token_dir, prev_token);
        strcpy(prev_token,token);
        token = strtok(NULL, "/"); 
        printf("TOKENDIR: %s\n", token_dir);
        printf("PrevTOKEN: %s\n", prev_token);
        printf("TOKEN: %s\n", token);

    } 
    printf("DIR NAME: %s\n", token_dir);
    return token_dir;
    //char* str = dirname(path);
    //printf("exDIR: %s\n", str);
    //return str;
}
//END OF HELPER FUNCTIONS

char *extract_name(char *path)
{
    printf("exname_PATH:%s\n", path);
    if(strcmp(path,"/")==0)
        return path;
    char str[strlen(path)];
    strcpy(str,path);
    char* token = strtok(str, "/"),*prev_token = malloc(sizeof(char)*strlen(path)); 
    while (token != NULL) { 
        strcpy(prev_token,token);
        token = strtok(NULL, "/"); 
        printf("PrevTOKEN: %s\n", prev_token);
        printf("TOKEN: %s\n", token);

    } 
    printf("NAME: %s\n", prev_token);
    return prev_token;
    //char *str = basename(path);
    //printf("exNAME: %s\n", str);
    //return str;
} 

void init_root()
{
    printf("Initializing Root\n");
    root = (Inode*)malloc(sizeof(Inode));
    root->path = "/";
    root->name = "/";
    root->type = 1;
    //Once persistence is acheived load files and update accordingly
    root->no_of_children = 0;
    root->size = 0;
    root->inode_num = GLOBAL_INUM;
    root->parent = NULL;
    root->children = NULL;
    root->file.data = "";
    root->file.size = 0;
    GLOBAL_INUM ++;
}

void init_node(const char * path, char * name, Inode *parent,int type)
{
    printf("Initializing Tree Node\n");
    Inode *new_node =(Inode*)malloc(sizeof(Inode));
    new_node->path = (char *)calloc(sizeof(char), strlen(path) + 1);
    new_node->name = (char *)calloc(sizeof(char), strlen(name) + 1);
    strcpy(new_node->path, (char *)path);
    strcpy(new_node->name, (char *)name);
    new_node->type =type;
    printf("Type: %d\n", new_node->type);
    new_node->no_of_children = 0;
    new_node->size = 0;
    new_node->inode_num = GLOBAL_INUM ++;;
    new_node->child_inode = NULL;
    new_node->parent = parent;
    new_node->children = NULL;
    new_node->file.data = NULL;
    new_node->file.size = 0;
    GLOBAL_INUM ++;
    printf("INUM INCREMENTED\n");
    insert_node(new_node);
  //  return new_node;    
}

//insert node function
void insert_node(Inode *node)
{
    printf("INsert node called\n");
    Inode *parent;
    parent = node->parent;
    //parent = searchNode(node);
    printf("parent: %s\n", parent->name);
    if(parent->children!=NULL)
    {
        parent->children=(Inode**)realloc(parent->children,sizeof(Inode*) * parent->no_of_children+1);
        parent->child_inode =(int*)realloc(parent->child_inode, sizeof(int)*parent->no_of_children+1);
    }
    else
    {
        parent->children=(Inode**)malloc(sizeof(Inode*));
        parent->child_inode = (int*)malloc(sizeof(int));
    }
    parent->children[parent->no_of_children] = node;
    parent->child_inode[parent->no_of_children]=node->inode_num;
    parent->no_of_children++;
    printf("NO OF CHILDREN OF %s IS %d", parent->name, parent->no_of_children);
    printf("Inserted node type: %d\n", parent->children[parent->no_of_children-1]->type);
    printf("Inserted node: %s\n", parent->children[parent->no_of_children-1]->name);
}
//end of insert node

//flag=0 means search based on name
//flag=1 means search based on inode numder
Inode* search(Inode *node, char *name,int inodenum,int flag)
{
    printf("Search Node\n");
    int i;
    Inode* result;
    int no_of_children = node->no_of_children;
    //printf("%s\n", name);
	
	if(flag==0 && name!=NULL && inodenum==-1)
	{
		if(!(strcmp(node->name,name)))
		{
			printf("NODE RETURNED\n");
			return node;
		}

		if(no_of_children == 0)
			return NULL;

		for(i = 0;i < no_of_children;i++)
		{
			if(node->children[i]->type == 1)
			{
				if(!(strcmp(node->children[i]->name,name)))
				{
						printf("CHILD RETURNED\n");
						printf("CHILD: %s\n", node->children[i]->name);
					return node->children[i];
				}

				result = search(node->children[i],name,-1,0);
				if(result != NULL)
					return result;
			}
			else
			{
				printf("CHILD TYPE 0\n");
				if(!(strcmp(node->children[i]->name,name)))
				{
					printf("CHILD NODE RETURNED\n");
					printf("CHILD NAME%s\n", node->children[i]->name);
					return node->children[i];
				}
			}
		}
	}
	else if(flag==1 && inodenum!=-1 && name==NULL)
	{
		if(node->inode_num==inodenum)
		{
			printf("NODE RETURNED\n");
			return node;
		}

		if(no_of_children == 0)
			return NULL;

		for(i = 0;i < no_of_children;i++)
		{
			if(node->children[i]->type == 1)
			{
				if(node->children[i]->inode_num==inodenum)
				{
						printf("CHILD RETURNED\n");
						printf("CHILD: %s\n", node->children[i]->name);
					return node->children[i];
				}

				result = search(node->children[i],NULL,inodenum,1);
				if(result != NULL)
					return result;
			}
			else
			{
				printf("CHILD TYPE 0\n");
				if(node->children[i]->inode_num,inodenum)
				{
					printf("CHILD NODE RETURNED\n");
					printf("CHILD NAME%s\n", node->children[i]->name);
					return node->children[i];
				}
			}
		}
	}
    return NULL;
}

char * read_data(Inode* node)
{
    printf("READ DATA\n");
    //Inode* inode = search(root, path);
    //int inode_number = temp->inode_num;
    printf("read_datafilesize: %d\n", node->file.size);
    char *temp = (char *)malloc(sizeof(char)*node->file.size+1);
    strcpy(temp,node->file.data);
    printf("read_datafilesize: %s\n", temp);
    //write_to_disk_wrapper();
    return temp;
}

int write_data(Inode* inode,char *data)
{
    //int inode_number = find_path(path);
    //printf("%d\n",inode_number );
    //inode_blocks[inode_number].size = strlen(data);
    //printf("hi\n");
    printf("WRITE DATA\n");
    printf("%d\n", inode->file.size);
    if(inode->file.data == NULL)
    {
        inode->file.size = strlen(data);
        printf("FILE SIZZE: %d STRLEN: %ld\n", inode->file.size, strlen(data));
        inode->file.data = (char*)malloc(inode->file.size * sizeof(char));
        strcpy(inode->file.data,data);
    }
    else
    {
        inode->file.size += strlen(data);
        inode->file.data = (char*)realloc(inode->file.data, inode->file.size * sizeof(char));
        strcat(inode->file.data, data);
    }
    return strlen(data);
}

//delete node
int deleteNode(const char *path)
{
    char *filename = extract_name(path);
    printf("REMOVING FILE: %s", filename);
    if(strcmp(filename,"/")==0)
    {
        printf("ERROR CANNOT DELETE ROOT DIRECTORY\n");
        return -1;
    }
    Inode *node = search(root,filename,-1,0);

    if(node==NULL)
    {
        printf("ERROR FILE NOT FOUND\n");
        return -1;
    }

    if(node->type==1 && node->no_of_children>0)
    {
        printf("ERROR CANNOT DELETE DIRECTORY WITH FILES IN IT\n");
        return -1;
    }

    int temp_inum = node->inode_num;
    char *dirname = extractDir(path);
    Inode *parent = search(root,dirname,-1,0);
    printf("rmparent: %s\n", parent->name);
    printf("rmparents children: %d\n children", parent->no_of_children);
    //write_to_disk_wrapper();
    int i, j;
    for(i=0; i<parent->no_of_children; i++)
    {
        printf("rminfor\n");
        if(parent->children[i]->inode_num == node->inode_num)
        {
            for(j=i; j<parent->no_of_children-1; j++)
            {
                parent->children[j] = parent->children[j+1];
                parent->child_inode[j] = parent->child_inode[j+1];
            }
        }
    }
    parent->no_of_children--;
    if(parent->no_of_children == 0)
    {
        parent->children = NULL;
    }
    /*else
    {
        parent->child_inode = (int*)realloc(parent->no_of_children, parent->no_of_children*sizeof(int));
        parent->children = (Inode**)realloc(parent->no_of_children, parent->no_of_children*sizeof(Inode*));
    }
    //int* new_child_inode = (int*)malloc(parent->no_of_children*sizeof(int));
    //int j=0;
    /*for(i=0;i<parent->no_of_children;i++)
    {
        if(parent->child_inode[i]!=temp_inum)
        {
            new_child_inode[j] = parent->child_inode[i];
            j++;
        }
        else
        {
            parent->children[i] = NULL;
        }   
    }*/
    //parent->child_inode = new_child_inode;
   //write_to_disk_wrapper();
    free(node);
    return 0; //success

}

static int sys_getattr(const char* path, struct stat *st)
{
    printf("Get Attribute Called\n");
    //Find inode of that file
    char* temp_path = path;
    printf("%s\n", path);
    char* temp_name = extract_name(temp_path);
    printf("getattr_tempname: %s\n", temp_name);

    Inode *I = search(root, temp_name,-1,0);//found inode
    printf("Searched\n");
    if(I == NULL)
        return -ENOENT;
    printf("getattr_name: %s\n", I->name);

    File F = I->file;
    if(I->type == 1)    //directory
    {
        st->st_mode = S_IFDIR | 0777;
        st->st_nlink = 2;
    }
    else if(I->type == 0)   //file
    {
        printf("Regular file\n");
        st->st_mode = S_IFREG | 0777;
        st->st_nlink = 1;
        st->st_size = F.size;
        st->st_blocks = (((st->st_size) / 512) + 1);
    }
    else
    {
        printf("Get att returned enoent\n");
        return -ENOENT;
    }
    st->st_nlink = I->no_of_children;
    st->st_atime = time(NULL);
    st->st_mtime = time(NULL);
    printf("Get Attribute over\n");
    return 0;
}

static int sys_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
    printf("Read Directory Called\n");
    filler(buffer, ".", NULL, 0);
    filler(buffer, "..", NULL, 0);
    Inode *I;
    int i;
    char* temp_path = path;
    if(strcmp(path, "/") == 0)
    {
        printf("IN IF\n");
        printf("PATH IS ROOT\n");
        I = root;
    }
    else
    {
        printf("IN ELSE\n");
        char* temp_name = extract_name(temp_path);
        printf("readdir_tempname: %s\n", temp_name);
        I = search(root, temp_name,-1,0);
        printf("PATH IS %s", I->name);//found inode
        printf("CHildren: %d\n",I->no_of_children );
    }
    if(I == NULL)
        return -ENOENT;

    for(i=0; i<I->no_of_children; i++)
        filler(buffer, I->children[i]->name, NULL, 0);
    return 0;
}

static int sys_mkdir(const char * path, mode_t x){
    int type = 1;
    char* temp_path = path;
    char* temp_dir = extractDir(temp_path);
    char* name = extract_name(temp_path);
    Inode* parent = search(root, temp_dir,-1,0);
    printf("mkdir_Parent: %s\n", parent->name);
    init_node(temp_path, name, parent, type);
    printf("Made directory\n");
    write_to_disk_wrapper();
    return 0;
}

static int sys_mknod(const char * path, mode_t x, dev_t y){
    printf("Make Node\n");
    int type = 0;
    char* temp_path = path;
    char* temp_dir = extractDir(temp_path);
    printf("mknode_tempdir: %s\n", temp_dir);
    char* name = extract_name(temp_path);
    Inode* parent = search(root, temp_dir,-1,0);
    printf("mknodPARENT: %s\n", parent->name);
    init_node(temp_path, name, parent, type);
    write_to_disk_wrapper();
    return 0;
}

static int sys_open(const char *path, struct fuse_file_info *fi)    //return inode number
{
    char* temp_path = path;
    char* temp_name = extract_name(temp_path);
    Inode* file = search(root, temp_name,-1,0);
    int file_pointer;
    if(file != NULL)
        file_pointer = file->inode_num;
    else
        file_pointer = -1;
    if(file_pointer < 0) {  
        fprintf(stderr, "Open()\t Error: File [%s] does not exist.\n",temp_name);
        return -1;
    }
    printf("Open()\t : File [%s] opened.\n", temp_name);
    return 0;
}

static int sys_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
    printf("Read called\n");
    char* temp_path = path;
    char* temp_name = extract_name(temp_path);
    Inode* temp = search(root, temp_name,-1,0);
    char * data=read_data(temp);
    printf("read_datafilesize: %s\n", data);
    memcpy(buffer,  data + offset, size);
    return size;
}

static int sys_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    printf("WRITE called and path is :%s\n",path);
    char* temp_path = path;
    char* temp_name = extract_name(temp_path);
    Inode* temp = search(root, temp_name,-1,0);
    printf("write_temp: %s\n", temp->name);
    if(temp == NULL){
        return 0;
    }
    write_data(temp,buf);
    write_to_disk_wrapper();
    return size;    
}

static int sys_rmdir(const char *path)
{
    printf("RMDIR CALLED for path %s\n",path);
    int ret = deleteNode(path);
    if(ret < 0)
        return -1;
    write_to_disk_wrapper();
    return 0;
}

static int sys_unlink(const char *path)
{
    printf("UNLINK CALLED for path %s\n",path);
    int ret = deleteNode(path);
    if(ret < 0)
        return -1;
    write_to_disk_wrapper();
    return 0;
}
int main( int argc, char *argv[] ){
    int i;
    /*for(i=0;i<MAX_BLOCKS;i++){
        i_bitmap[i]=0;
        d_bitmap[i]=0;
    }
    make_node("/","d");
    find_path("/");
    make_node("/abc.txt","f");
    write_data("/abc.txt","hello world");*/
    //global FILE *mfp = fopen("/home/hduser/Desktop/2.txt","w+");
    //printf("%s\n",file_blocks[inode_blocks[1].blk_no[0]].data);
    //printf("%s\n",read_data("/abc.txt"));
    //return 0; 
//  read_from_disk();
    //printf("d_bitmap %d\n",i_bitmap[0]);
    init_root();
    return fuse_main( argc, argv, &operations);
}