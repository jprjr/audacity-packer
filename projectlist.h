#ifndef PROJECTLIST_H
#define PROJECTLIST_H

struct projectlist_s {
    char **projects;
    unsigned int total;
};

typedef struct projectlist_s projectlist;

#ifdef __cplusplus
extern "C" {
#endif

projectlist *projectlist_new(void);
void projectlist_free(projectlist *plist);
void projectlist_append(projectlist *plist, const char *path);
int projectlist_fill(projectlist *plist, const char *path);

#ifdef __cplusplus
}
#endif

#endif
