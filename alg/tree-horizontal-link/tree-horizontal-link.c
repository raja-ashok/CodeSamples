#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SUCCESS 0
#define FAILURE -1

typedef struct tree_node_st TREE_NODE;

struct tree_node_st {
  int num;
  TREE_NODE *left;
  TREE_NODE *right;
  TREE_NODE *link;
};

typedef struct tree_node_data_st TREE_NODE_DATA;

struct tree_node_data_st {
  TREE_NODE *node;
  int depth; /* At what depth this node is present */
};

typedef struct linked_list_st LLIST;

struct linked_list_st {
  TREE_NODE_DATA tree_node;
  void *next;
};

/* root holds root node of the tree */
TREE_NODE *root = NULL;
int depth;

/* nodes_at_level holds the first node of each level */
/* Used to print the horizontal link */
TREE_NODE **nodes_at_level = NULL;

void insert_new_node_to_tree(TREE_NODE *new) {
  int level = 0;
  TREE_NODE *cur = root;
  if (root == NULL) {
    root = new;
    depth = 0;
    return;
  }
  while (cur != NULL) {
    level++;
    if (new->num < cur->num) {
      if (cur->left == NULL) {
        cur->left = new;
        break;
      }
      cur = cur->left;
    } else if (cur->num < new->num) {
      if (cur->right == NULL) {
        cur->right = new;
        break;
      }
      cur = cur->right;
    }
  }
  if (level > depth) {
    depth = level;
  }
}

int update_tree(int num) {
  TREE_NODE *new;
  if (num <= 0) {
    printf("Ignoring zero or neg number[%d]\n", num);
    return SUCCESS;
  }
  if ((new = (TREE_NODE *)malloc(sizeof(TREE_NODE))) == NULL) {
    printf("Mem alloc failed\n");
    return FAILURE;
  }
  memset(new, 0, sizeof(TREE_NODE));
  new->num = num;
  insert_new_node_to_tree(new);
  return SUCCESS;
}

int create_tree(char *num_str) {
  char *token, *rest;
  rest = num_str;
  while ((token = strtok_r(rest, ",", &rest)) != NULL) {
    printf("Adding [%s] to tree\n", token);
    if (update_tree(atoi(token)) == FAILURE) {
      printf("Failed for num[%s]\n", token);
      return FAILURE;
    }
  }
  printf("Created tree with depth[%d]\n", depth);
  return SUCCESS;
}

void print_tree_preorder(TREE_NODE *node) {
  if (node == NULL) return;
  printf("%d  ", node->num);
  print_tree_preorder(node->left);
  print_tree_preorder(node->right);
}

void print_tree() {
  /* Recursive preorder print operation */
  printf("\nPreorder Print: \n");
  print_tree_preorder(root);
  printf("\n");
}

void free_tree_postorder(TREE_NODE *node) {
  if (node == NULL) return;
  free_tree_postorder(node->left);
  free_tree_postorder(node->right);
  free(node);
}

void free_tree() {
  printf("Releasing tree mem\n");
  free_tree_postorder(root);
}

int alloc_level_node_list(TREE_NODE ***level_node_list) {
  if ((*level_node_list = (TREE_NODE **)malloc(sizeof(TREE_NODE *) * (depth+1)))
                                                                  == NULL) {
    printf("Mem alloc failed for level last visited\n");
    return FAILURE;
  }
  memset(*level_node_list, 0, (sizeof(TREE_NODE *) * (depth+1)));
  return SUCCESS;
}

int prepend_llist(LLIST **list, TREE_NODE *node, int level) {
  LLIST *new;

  if ((new = (LLIST *)malloc(sizeof(LLIST))) == NULL) {
    printf("Mem alloc failed for llist\n");
    return FAILURE;
  }
  memset(new, 0, sizeof(LLIST));
  new->tree_node.node = node;
  new->tree_node.depth = level;
  new->next = *list;
  *list = new;
  printf("Prepended [%d] to list\n", node->num);
  return SUCCESS;
}

LLIST *get_first_from_llist(LLIST **list) {
  LLIST *first;

  if (*list == NULL) return NULL;
  first = *list;
  *list = first->next;
  printf("Removed [%d] from list\n", first->tree_node.node->num);
  return first;
}

void free_list(LLIST *list) {
  LLIST *cur = list;
  LLIST *next;
  printf("Releasing mem on list[%p]\n", list);
  while (cur != NULL) {
    next = cur->next;
    free(cur);
    cur = next;
  }
}

int update_horizontal_link() {
  /* The right nodes are kept always during preorder traversal, so that
   * we can come back and revisit those pending right nodes in traversal */
  LLIST *revisit_nodes = NULL;
  /* Stores the last visited node on each level, helps to update horizontal
   * link */
  TREE_NODE **level_last_visited = NULL;
  LLIST *list_node;
  TREE_NODE *cur;
  int level = 0;
  int ret_val = FAILURE;

  if ((alloc_level_node_list(&level_last_visited) == FAILURE)
        || (alloc_level_node_list(&nodes_at_level) == FAILURE)) {
    goto err;
  }

  printf("\nUpdating horizontal link\n");

  /* Below loop updates horizontal links on each level of a binary tree
   * without using any recursive calls */
  cur = root;
  while (cur != NULL) {
    printf("Curr node[%d] at level [%d]\n", cur->num, level);
    /* 1. Update cur and also at level_last_visited */
    if (cur->left != NULL) {
      cur->left->link = cur->right;
    }
    if (nodes_at_level[level] == NULL) {
      if (cur->left != NULL) {
        nodes_at_level[level] = cur->left;
      } else if (cur->right != NULL) {
        nodes_at_level[level] = cur->right;
      }
    }
    if (level_last_visited[level] != NULL) {
      if (cur->left != NULL) {
        level_last_visited[level]->link = cur->left;
      } else if (cur->right != NULL) {
        level_last_visited[level]->link = cur->right;
      }
    }
    if (cur->right != NULL) {
      level_last_visited[level] = cur->right;
    } else if (cur->left != NULL) {
      level_last_visited[level] = cur->left;
    }

    /* 2. Keep right for revisiting */
    if ((cur->left != NULL) && (cur->right != NULL)) {
      printf("Adding right node [%d] to revisit list\n", cur->right->num);
      if (prepend_llist(&revisit_nodes, cur->right, level + 1) == FAILURE) {
        printf("Prepending to list failed\n");
        goto err;
      }
    }

    /* 3. Move to next child if there is a child or else move to last inserted
     * node from revisit_nodes */
    if (cur->left != NULL) {
      cur = cur->left;
      level++;
    } else if (cur->right != NULL) {
      cur = cur->right;
      level++;
    } else {
      if ((list_node = get_first_from_llist(&revisit_nodes)) == NULL) {
        break;
      }
      cur = list_node->tree_node.node;
      level = list_node->tree_node.depth;
      free(list_node);
    }

    if ((level < 0) || (level > depth)) {
      printf("Abnormal Level[%d] for depth[%d]\n", level, depth);
      goto err;
    }
  }
  ret_val = SUCCESS;
err:
  free(level_last_visited);
  free_list(revisit_nodes);
  return ret_val;
}

void print_horizontal_link() {
  TREE_NODE *cur;
  int i;
  printf("\nPrinting Tree nodes horizontal link\n");
  for (i = 0; i <= depth; i++) {
    cur = nodes_at_level[i];
    if (cur == NULL) continue;
    printf("At depth[%d]:", i + 1);
    while (cur != NULL) {
      printf("%d-->", cur->num);
      cur = cur->link;
    }
    printf("\n");
  }
}

void free_horizontal_link() {
  free(nodes_at_level);
}

void usage() {
  printf("usage: <exe> <comma_separated_numbers_of_tree>\n");
  printf("<exe> 15,9,4,20,3,45,88,7\n");
  exit(0);
}

int main(int argc, char **argv) {
  int ret_val = FAILURE;
  if (argc < 2) {
    usage();
  }
  if (create_tree(argv[1]) == FAILURE) {
    goto err;
  }
  print_tree();
  if (update_horizontal_link() == FAILURE) {
    goto err;
  }
  print_horizontal_link();
  ret_val = SUCCESS;
err:
  free_tree();
  free_horizontal_link();
  return ret_val;
}
