#include "c_rbtree.h"
#include "stdio.h"

#define print(a) printf("%d\n", a);
#define corr(head) print(rbtree_is_correct(head, 0, 0))

int main() {
    int array[] = {1, 2, 4, 6, 3, 5};
    rbtree_t tree = init_rbtree();
    corr(tree);
    for (int i = 0; i < 6; i++) {
        rbtree_add_node(tree, array[i], NULL, 0);
        corr(tree);
    }
    for (int i = 5; i > -1; i--) {
        rbtree_node_t *tmp = rbtree_get_node(tree, array[i]);
        printf("0x%p\n", tmp);
        rbtree_delete_node(tree, tmp);
        corr(tree);
    }
    return 0;
}
