
#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_any_obj.h>
#include <aroop/opp/opp_str2.h>
#include <aroop/aroop_memory_profiler.h>

typedef struct avlnode {
	int key;
	int height;
	struct avlnode*right;
	struct avlnode*left;
	struct avlnode*equals;
	struct avlnode*parent;
} avlnode_t;

/**
 * The AVL tree keeps an extra property of the node. It is the heght of the node.
 * 
 * - Height is defined as the longest path to the leaf. So the height of a leaf is 0.
 * - Height of an internal node is max(height of children)
 */
void avlnode_fixHeight(avlnode_t*node) {
	node->height = 0;
	if(node->right && (node->right->height > (node->height-1))) {
		node->height = node->right->height+1;
	}
	if(node->left && (node->left->height > (node->height-1))) {
		node->height = node->left->height+1;
	}
	//cout << " height of " << key << " is " << height << endl;
	if(node->parent)
		avlnode_fixHeight(node->parent);
}

avlnode_t*avlnode_rotateRight(avlnode_t*node) {
	assert(node->left != NULL);
	//cout << " rotating right " << key << endl;
	avlnode_t*head = node->left;
	head->parent = node->parent;
	if(node->parent) {
		if(node->parent->right == node) {
			//OPPUNREF(node->parent->right);
			node->parent->right = head;
		} else {
			//OPPUNREF(node->parent->left);
			node->parent->left = head;
		}
	}

	// transplant left
	//OPPUNREF(node->left);
	node->left = head->right;
	if(node->left)
		node->left->parent = node;

	// transplant right
	head->right = node;
	node->parent = head;

	// fix height property
	avlnode_fixHeight(node);
	return head;
}

avlnode_t*avlnode_rotateLeft(avlnode_t*node) {
	assert(node->right != NULL);
	//cout << " rotating left " << key << endl;
	avlnode_t*head = node->right;
	head->parent = node->parent;

	if(node->parent) {
		//cout << " adding link to parent " << parent->key << endl;
		if(node->parent->right == node)
			node->parent->right = head;
		else
			node->parent->left = head;
	}

	// transplant right
	node->right = head->left;
	if(node->right)
		node->right->parent = node;

	// transplant head->left
	head->left = node;
	node->parent = head;

	// fix height property
	avlnode_fixHeight(node);

	return head;
}

avlnode_t*avlnode_fixAVLProperty(avlnode_t*node) {
	int rheight = (node->right?node->right->height:-1);
	int lheight = (node->left?node->left->height:-1);
	int diff = lheight - rheight;
	avlnode_t*head = node;

	//cout << " checking avl property for " << key << " the diff is " << diff << endl;
	/* check AVL condition */
	if(diff == 1 || diff == -1 || diff == 0)
		return head; // no worry

	if(diff > 0) { /* if left tree is too deep */
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
	return head;
}

avlnode_t*avlnode_insert(avlnode_t*node, avlnode_t*target) {
	if(target->key < node->key) {
		if(node->left)
			avlnode_insert(node->left, target);
		else {
			//node->left = new AVLNode(aKey);
			node->left = target;
			node->left->parent = node;
			avlnode_fixHeight(node);
		}
	} else if(node->key == target->key) {
		if(node->equals)
			return avlnode_insert(node->equals, target);
		else
			node->equals = target;
		return node;
	} else {
		if(node->right)
			avlnode_insert(node->right, target);
		else {
			node->right = target;
			node->right->parent = node;
			avlnode_fixHeight(node);
		}
	}
	//cout << "inserted " << aKey << endl;
	return avlnode_fixAVLProperty(node);
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

void avlnode_dump(avlnode_t*node) { // prints tree in preorder
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
	avlnode_t*root = NULL;
	int i = n;
	srand(x);
	while(i--) {
		int r = rand();
		avlnode_t*node = OPP_ALLOC2(&node_factory, &r);
		if(!root)
			root = node;
		else {
			avlnode_t*new_root = avlnode_insert(root, node);
			if(new_root)
				root = new_root;
		}
	}
	if(n < 20) {
		avlnode_dump(root);
		printf("\n");
		avlnode_in_order(root, in_order_dump_cb);
		printf("\n");
	}
	if(avlnode_search(root, x) != NULL) {
		printf("found %d \n", x);
	} else {
		printf("not found %d \n", x);
	}
	OPPUNREF(root);
	assert(OPP_FACTORY_USE_COUNT(&node_factory) == 0);
	return 0;
}

int binary_tree_test(int argc, char*argv[]) {
	OPP_FACTORY_CREATE(&node_factory, 128, sizeof(struct avlnode), OPP_CB_FUNC(avlnode));
	test_search(15, 3);
	test_search(1000, 3343);
	test_search(9000, 234);
	test_search(5000, 543);
	test_search(9000, 234);
	test_search(8000, 456);
	return 0;
}

