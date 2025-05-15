#include "../c_list.h"
#include <stdio.h>

typedef unsigned char MyType;

typedef struct {
    node_t list;
    MyType data;
} MyList;

int main() {
    printf("Hello world.\n");
    list_t head = init_list_default();
    printf("%p\n", head);
    MyType a = 114, b = 255;
    push_end(head, &a, sizeof(MyType));
    printf("%p\n", head);
    push_end(head, &b, sizeof(MyType));
    printf("%p\n", head);
    printf("a: %d, b: %d\n", ((MyList *) (head->node.next))->data,
           ((MyList *) (((MyList *) (head->node.next))->list.next))->data);
}
