#include "table.h"

LHASH *first;
LHASH *second;

void init_stat() {
    stat.request = 0;
    stat.response = 0;
    stat.retransmit = 0;
    stat.dropped = 0;
    stat.time = global_timestamp;
}

void print_stat() {
    write_stat("request=%d retransmit=%d response=%d no-response=%d dropped=%d", 
	       stat.request, stat.retransmit, stat.response, stat.not_response, stat.dropped);
}

unsigned long lh_hash_cb(config_record_t *r) {
  char *a;
  unsigned long h = 0;
  int c = 0;
  for (a = r->key; *a && c < 32; a++, c++) {
    h ^= (unsigned long)(*a);
    h <<= 1;
  }
  return h;
}

int lh_cmp_cb(config_record_t *a, config_record_t *b) {
  return strcmp(a->key, b->key);
}

void lh_dump_all_cb(config_record_t *a) {
    write_error("Error");
}

void lh_clean_all_cb(config_record_t *a, char *arg) {    
    static const char* proc = "clean table";
    int *ar;
    int curr;
    config_record_t *b1, *b2;
    char *tmp_key;
    char str[20];
    char ip[16];
    char port[6];
    char id[4];  
    char *p_ip, *p_port, *p_id;
    int i;
    int flag = 1;
    ar = (int*)arg;
    curr = *ar;
    
    *ip = '\0';
    *port = '\0';
    *id = '\0';
    
    p_ip = ip;
    p_port = port;
    p_id = id;      

    if (((second_key*)(a->first))->flag == 1) {
	stat.not_response++;
    }

    if (curr - ((second_key*)(a->first))->time >= exp_time) {
	b1 = b2 = (config_record_t*)NULL;
	
	b1 = drop_record_first(a->key);	
	tmp_key = (char*)malloc(strlen(a->key) + 1);
	*tmp_key = '\0';	
	strcpy(tmp_key, a->key);

	if (b1) {
	    sprintf(str, "%d_%d", b1->first->radclnt, b1->first->id);	
	    b2 = drop_record_second(str);	    
	    free(b1->first);
	    free(b1->key);
	    free(b1);
	} else {
	    write_error("%s pointer first key is NULL", proc); 
	}

	if (b2) {
	    free(b2->second);
	    free(b2->key);
	    free(b2);
	} else {
	    write_error("%s free second key", proc);
	}

	for(i = 0; i < strlen(tmp_key); i++) {
	    if (tmp_key[i] == '_') {
		flag++;
	    } else if (flag == 1) {
		*p_ip = tmp_key[i];
		p_ip++;
		*p_ip = '\0';
	    } else if (flag == 2) {
		*p_port = tmp_key[i];
		p_port++;
		*p_port = '\0';
	    } else if (flag == 3) {
		*p_id = tmp_key[i];
		p_id++;
		*p_id = '\0';
	    }	
	}
	write_info("drop record for [ip = %s port = %s id = %s]",
		   ip, port, id);    	
	free(tmp_key);
    }    
}

#ifdef STD_OPENSSL
static IMPLEMENT_LHASH_HASH_FN(lh_hash_cb, config_record_t *);
static IMPLEMENT_LHASH_COMP_FN(lh_cmp_cb, config_record_t *);
static IMPLEMENT_LHASH_DOALL_FN(lh_dump_all_cb, config_record_t *);
static IMPLEMENT_LHASH_DOALL_FN(lh_clean_all_cb, config_record_t *);
#endif

int reg_record_first(char *key, second_key *data) {
  config_record_t a, *b;
  a.key = key;
#ifdef STD_OPENSSL
  b = lh_retrieve(first, &a);
#else
  b = (config_record_t *)lh_retrieve(first, (char *)&a);
#endif
  if (b == NULL) {
    b = malloc(sizeof(config_record_t));
    b->key  = (char*)malloc(strlen(key)+1);
    b->first = (second_key*)malloc(sizeof(second_key));
    strcpy(b->key, key);
    (b->first)->radclnt = data->radclnt;
    (b->first)->id = data->id;
    (b->first)->time = data->time;
    (b->first)->flag = data->flag;
#ifdef STD_OPENSSL
    lh_insert(first, b);
#else
    lh_insert(first, (char *)b);
#endif
  } else {
      //if exist the same key
      return 1;
  }
  return 0;
}

int reg_record_second(char *key, char *data) {
  config_record_t a, *b;
  a.key = key;
#ifdef STD_OPENSSL
  b = lh_retrieve(second, &a);
#else
  b = (config_record_t *)lh_retrieve(second, (char *)&a);
#endif
  if (b == NULL) {
      b = malloc(sizeof(config_record_t));
      b->key  = (char*)malloc(strlen(key)+1);
      b->second = (char*)malloc(strlen(data)+1);
      strcpy(b->key, key);
      strcpy(b->second, data);      
#ifdef STD_OPENSSL
      lh_insert(second, b);
#else
      lh_insert(second, (char *)b);
#endif      
  } else {
      //if exist the same key
      return 1;
  }
  return 0;
}

config_record_t *drop_record_first(char *key) {
    config_record_t a, *b;
    a.key = key;
#ifdef STD_OPENSSL
    b = lh_delete(first, &a);
#else
    b = (config_record_t *)lh_delete(first, (char *)&a);
#endif
    if (b == NULL) {      
	return NULL;
    } else {
	return b;
    }
}

config_record_t *drop_record_second(char *key) {
    config_record_t a, *b;
    a.key = key;
#ifdef STD_OPENSSL
    b = lh_delete(second, &a);
#else
    b = (config_record_t *)lh_delete(second, (char *)&a);
#endif
    if (b == NULL) {      
	return NULL;
    } else {
	return b;
    }
}

second_key *get_record_first(char *key) {
  config_record_t a, *b;
  a.key = key;
#ifdef STD_OPENSSL
  b = lh_retrieve(first, &a);
#else
  b = (config_record_t *)lh_retrieve(first, (char *)&a);
#endif
  if (b == NULL) {      
      return NULL;
  } else {
      return b->first;
  }
}

char *get_record_second(char *key) {
  config_record_t a, *b;
  a.key = key;
#ifdef STD_OPENSSL
  b = lh_retrieve(second, &a);
#else
  b = (config_record_t *)lh_retrieve(second, (char *)&a);
#endif
  if (b == NULL) {      
      return NULL;
  } else {
      return b->second;
  }
}

void clean_table() {
    int curr_t;
    curr_t = global_timestamp;    
    stat.not_response = 0;
#ifdef STD_OPENSSL
    lh_doall_arg(first, LHASH_DOALL_FN(lh_clean_all_cb), (char*)&curr_t);
#else
    lh_doall_arg(first, lh_clean_all_cb, (char*)&curr_t);
#endif   
    if (stat.time == 0) {
	init_stat();
    }
    if (curr_t >= exp_time + stat.time) {
	print_stat();
	init_stat();	
    }    
}

void init_table_first() {    
#ifdef STD_OPENSSL
  first = lh_new(LHASH_HASH_FN(lh_hash_cb), LHASH_COMP_FN(lh_cmp_cb));
#else
  first = lh_new(lh_hash_cb, lh_cmp_cb);
#endif
}

void init_table_second() {
#ifdef STD_OPENSSL
  second = lh_new(LHASH_HASH_FN(lh_hash_cb), LHASH_COMP_FN(lh_cmp_cb));
#else
  second = lh_new(lh_hash_cb, lh_cmp_cb);
#endif
}
