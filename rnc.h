/* $Id$ */

#ifndef RNC_H
#define RNC_H 1

struct rnc_source;
extern struct rnc_source *rnc_alloc(void);
extern void rnc_free(struct rnc_source *sp);

extern void rnc_init(void);
extern void rnc_clear(void);

extern int rnc_open(struct rnc_source *sp,char *fn);
extern int rnc_stropen(struct rnc_source *sp,char *fn,char *s,int len);
extern int rnc_bind(struct rnc_source *sp,char *fn,int fd);
extern int rnc_close(struct rnc_source *sp);

extern int rnc_parse(struct rnc_source *sp);

extern int rnc_errors(struct rnc_source *sp);

#endif
