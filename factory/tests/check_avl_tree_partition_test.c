
#include <check.h>
#include "aroop/core/config.h"
#include "aroop/core/memory.h"
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_list.h"
#include "aroop/opp/opp_queue.h"
#include "aroop/core/logger.h"
#include "aroop/opp/opp_watchdog.h"
#include "aroop/opp/opp_iterator.h"
#include "aroop/opp/opp_factory_profiler.h"

C_CAPSULE_START

#include "../src/opp_factory_part_property.c"

typedef struct avlnode {
	int key;
	int height;
	struct avlnode*right;
	struct avlnode*left;
	struct avlnode*equals;
	struct avlnode*parent;
} avlnode_t;

typedef struct avltree {
	struct avlnode*root; /* it is the root node */
} avltree_t;

/**
 * The AVL tree keeps an extra property of the node. It is the heght of the node.
 * 
 * - Height is defined as the longest path to the leaf. So the height of a leaf is 0.
 * - Height of an internal node is max(height of children)
 */
static inline void avlnode_fixHeight(avlnode_t*node) {
	ck_assert(node);
	node->height = 0;
	if(node->right && (node->right->height > (node->height-1))) {
		node->height = node->right->height+1;
		ck_assert(node->right->parent == node);
	}
	if(node->left && (node->left->height > (node->height-1))) {
		node->height = node->left->height+1;
		ck_assert(node->left->parent == node);
	}
	//cout << " height of " << key << " is " << height << endl;
	if(node->parent)
		avlnode_fixHeight(node->parent);
}

static inline void avlnode_set_parent(avlnode_t* const node, avlnode_t* const parent) {
	/* Avoid incrementing the reference count of the parent(weak). Otherwise there will be circular reference. */
	node->parent = parent;
}


static inline void avlnode_set_right(avlnode_t* const node, avlnode_t* const right) {
	ck_assert(node);
	if(node->right)
		OPPUNREF(node->right);
	if(right) {
		node->right = OPPREF(right);
		avlnode_set_parent(node->right, node);
		struct opp_object*obj = data_to_opp_object(right);
		ck_assert(obj->refcount < 10);
	}
}

static inline void avlnode_set_left(avlnode_t* const node, avlnode_t* const left) {
	ck_assert(node);
	if(node->left)
		OPPUNREF(node->left);
	if(left) {
		node->left = OPPREF(left);
		avlnode_set_parent(node->left, node);
	}
}

static inline avlnode_t*avlnode_rotateRight(avlnode_t*const node) {
	ck_assert(node->left != NULL);
	//cout << " rotating right " << key << endl;
	avlnode_t* const head = OPPREF(node->left);
	ck_assert(head->parent == node);
	avlnode_set_parent(head, node->parent);

	// rotate
	avlnode_set_left(node, head->right);
	avlnode_set_right(head, node);

	/* fix parent */
	if(head->parent) {
		/* if node is not root */
		if(head->parent->right == node)
			avlnode_set_right(head->parent, head);
		else
			avlnode_set_left(head->parent, head);
		OPPUNREF(head);
	} else {
		avlnode_fixHeight(node);
		OPPUNREF(node);
	}

	// fix height property
	if(node)
		avlnode_fixHeight(node);
	return head;
}

static inline avlnode_t*avlnode_rotateLeft(avlnode_t* const node) {
	ck_assert(node->right != NULL);
	//cout << " rotating left " << key << endl;
	avlnode_t* const head = OPPREF(node->right);
	ck_assert(head);
	ck_assert(head->parent == node);
	//head->parent = node->parent;
	avlnode_set_parent(head, node->parent);

	avlnode_set_right(node, head->left);
	avlnode_set_left(head, node);

	/* fix parent */
	if(head->parent) {
		/* if node is not root */
		//cout << " adding link to parent " << parent->key << endl;
		if(head->parent->right == node)
			avlnode_set_right(head->parent, head);
		else
			avlnode_set_left(head->parent, head);
		OPPUNREF(head);
	} else {
		avlnode_fixHeight(node);
		OPPUNREF(node);
	}

	// fix height property
	if(node)
		avlnode_fixHeight(node);
	return head;
}

static inline void avlnode_fixAVLProperty(avltree_t*tree, avlnode_t*node) {
	const int rheight = (node->right?node->right->height:-1);
	const int lheight = (node->left?node->left->height:-1);
	int diff = lheight - rheight;
	avlnode_t*head = node;

	//cout << " checking avl property for " << key << " the diff is " << diff << endl;
	/* check AVL condition */
	if(diff == 1 || diff == -1 || diff == 0)
		return; // no worry

	if(diff > 0) { /* if left tree is too deep */
		ck_assert(node->left != NULL);
		if(!node->left->left && node->left->right) { // if the left child has no more left child and if there is right child
			avlnode_rotateLeft(node->left); // so the left child has only left child (no right child), kind of straight line
		}
		head = avlnode_rotateRight(node); // rotate it right to lower the left height
	} else { /* right tree is too deep */
		if(!node->right->right && node->right->left) { // if it is a tangle
			avlnode_rotateRight(node->right); // make it straight.
		}
		head = avlnode_rotateLeft(node); // now rotate left to lower the right height
	}

	if(head && head->parent == NULL) { /* if it is the root */
		tree->root = head;
	}
}

void avlnode_insert(avltree_t* const tree, avlnode_t*node, avlnode_t* const target) {
	if(node == NULL) {
		node = tree->root;
		if(node == NULL) {
			tree->root = target;
			return;
		}
	}
	/* do simple binary tree insertion */
	if(target->key < node->key) {
		/* the smaller goes left */
		if(node->left)
			avlnode_insert(tree, node->left, target);
		else {
			//node->left = new AVLNode(aKey);
			node->left = target;
			avlnode_set_parent(target, node);
			avlnode_fixHeight(node);
		}
	} else if(node->key == target->key) {
		/* equal falls into equal */
		if(node->equals)
			avlnode_insert(tree, node->equals, target);
		else
			node->equals = target;
		return; /* no fixing required */
	} else {
		/* bigger goes right */
		if(node->right)
			avlnode_insert(tree, node->right, target);
		else {
			node->right = target;
			avlnode_set_parent(target, node);
			avlnode_fixHeight(node);
		}
	}
	//cout << "inserted " << aKey << endl;
	avlnode_fixAVLProperty(tree, node);
}

avlnode_t*avlnode_search(avlnode_t*node, int given) {
	if(node->key == given)
		return node;
	if(given < node->key) {
		if(!node->left) {
			return NULL;
		}
		return avlnode_search(node->left, given);
	}
	if(!node->right) {
		return NULL;
	}
	return avlnode_search(node->right, given);
}
// TODO remove node

static void avlnode_dump(avlnode_t*node) { // prints tree in preorder
	printf("%d", node->key);
	printf("(");
	if(node->left)
		avlnode_dump(node->left);
	printf(",");
	if(node->right)
		avlnode_dump(node->right);
	printf(")");
}

OPP_CB(avlnode) {
	struct avlnode*node = data;
	switch(callback) {
		case OPPN_ACTION_INITIALIZE:
			node->key = *((int*)cb_data);
			node->right = node->left = node->equals = node->parent = NULL;
			node->height = 0;
		break;
		case OPPN_ACTION_FINALIZE:
			if(node->equals) OPPUNREF(node->equals);
			if(node->left) OPPUNREF(node->left);
			if(node->right) OPPUNREF(node->right);
		break;
	}
	return 0;
}

void avlnode_in_order(avlnode_t*node, int (*cb)(int key)) { // traverses tree in in-order
	if(node->left)
		avlnode_in_order(node->left, cb);
	cb(node->key);
	if(node->right)
		avlnode_in_order(node->right, cb);
}

static struct opp_factory node_factory;

static int in_order_dump_cb(int key) {
	printf("%d,", key);
	return 0;
}

static int test_search(const int n, const int x) {
	avltree_t tree = {.root = NULL};
	int i = n;
	srand(x);
	while(i--) {
		const int r = rand();
		avlnode_t*node = OPP_ALLOC2(&node_factory, &r);
		ck_assert(node);
		ck_assert_int_eq(node->key, r);
		ck_assert_int_eq(node->height, 0);
		ck_assert(node->right == NULL);
		ck_assert(node->left == NULL);
		ck_assert(node->parent == NULL);
		ck_assert(node->equals == NULL);
		avlnode_insert(&tree, NULL, node);
		ck_assert(tree.root);
		ck_assert(tree.root->parent == NULL);
	}
	if(n < 20) {
		avlnode_dump(tree.root);
		printf("\n");
		avlnode_in_order(tree.root, in_order_dump_cb);
		printf("\n");
	}
	if(avlnode_search(tree.root, x) != NULL) {
		printf("found %d \n", x);
	} else {
		printf("not found %d \n", x);
	}
	OPPUNREF(tree.root);
	ck_assert(OPP_FACTORY_USE_COUNT(&node_factory) == 0);
	return 0;
}

START_TEST (binary_tree_test)
{
	OPP_FACTORY_CREATE(&node_factory, 1024, sizeof(struct avlnode), OPP_CB_FUNC(avlnode));
	test_search(15, 3);
	//test_search(1000, 3343);
	return 0;
}
END_TEST


Suite * binary_tree_test_suit(void) {
	Suite *s;
	TCase *tc_core;
	s = suite_create("check_avl_tree_partition_test.c");

	/* Core test case */
	tc_core = tcase_create("factory");

	tcase_add_test(tc_core, binary_tree_test);
	suite_add_tcase(s, tc_core);

	return s;
}


int main() {
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = binary_tree_test_suit();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

C_CAPSULE_END
