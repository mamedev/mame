/*********************************************************************

    simple_set.h

    A STL-like set class.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __SIMPLE_SET_H__
#define __SIMPLE_SET_H__

#ifdef SIMPLE_SET_DEBUG
#include <iostream>
#endif


// Predeclarations
template <class T> class avl_tree_node;
template <class T> class simple_set_iterator;


//
// ======================> simple_set
// A shiny stl-like set interface wrapping an AVL tree
//
// PUBLIC OPERATIONS:
// size, empty, clear, insert, remove, find, contains, merge, & assignment.
//

template <class T>
class simple_set
{
	friend class simple_set_iterator<T>;
	typedef avl_tree_node<T> tree_node;

public:
	// Construction
	simple_set()
		: m_root(NULL)
	{ }

	simple_set(const simple_set& rhs)
		: m_root(NULL)
	{
		*this = rhs;
	}

	~simple_set()
	{
		clear();
	}


	// Returns number of elements in the tree -- O(n)
	int size() const
	{
		if (empty()) return 0;

		const tree_node* currentNode = m_root;
		const int nodeCount = sizeRecurse(currentNode);
		return nodeCount;
	}


	// Test for emptiness -- O(1).
	bool empty() const
	{
		return m_root == NULL;
	}


	// Empty the tree -- O(n).
	void clear()
	{
		clearRecurse(m_root);
	}


	// Insert x into the avl tree; duplicates are ignored -- O(log n).
	bool insert(const T& x)
	{
		bool retVal = insert(x, m_root);

		// Whether the node was successfully inserted or not (i.e. wasn't a duplicate)
		return retVal;
	}


	// Remove x from the tree. Nothing is done if x is not found -- O(n).
	bool remove(const T& x)
	{
		// First find the node in the tree
		tree_node* currNode = find(x, m_root);

		// Only do this when the current node is valid
		if (currNode)
		{
			// See if it's a leaf
			if (currNode->isLeaf())
			{
				// Get the parent object
				tree_node* parentNode = currNode->parent;

				// If we're a leaf and we have no parent, then the tree will be emptied
				if (!currNode->parent)
				{
					m_root = NULL;
				}

				// If it's a leaf node, simply remove it
				removeNode(currNode);
				global_free(currNode);

				// Update the balance of the parent of
				// currentNode after having disconnected
				// it
				if (parentNode)
					parentNode->calcHeights();
			}
			else
			{
				// Get the parent object
				tree_node* parentNode = currNode->parent;

				// Remove the child and reconnect the smallest node in the right sub tree
				// (in order successor)
				tree_node* replaceNode = findMin(currNode->right);

				// See if there's even a right-most node
				if (!replaceNode)
				{
					// Get the largest node on the left (because the right doesn't exist)
					replaceNode = findMax(currNode->left);
				}

				tree_node* parentReplaceNode = replaceNode->parent;

				// Disconnect the replacement node's branch
				removeNode(replaceNode);

				// Update the balance of the parent of
				// replaceNode after having disconnected
				// it
				if (parentReplaceNode)
					parentReplaceNode->calcHeights();

				// Disconnect the current node
				removeNode(currNode);

				// Get the current node's left and right branches
				tree_node* left = currNode->left;
				tree_node* right = currNode->right;

				// We no longer need this node
				global_free(currNode);

				// Check to see if we removed the root node
				if (!parentNode)
				{
					// Merge the branches into the parent node of what we deleted
					merge(replaceNode, parentNode);
					merge(left, parentNode);
					merge(right, parentNode);

					// Now we're the root
					m_root = parentNode;
				}
				else
				{
					// Merge the branches into the parent node of what we
					// deleted, we let the merge algorithm decide where to
					// put the branches
					merge(replaceNode, parentNode);
					merge(left, parentNode);
					merge(right, parentNode);
				}
			}

			// Balance the tree
			balanceTree();

			// The node was found and removed successfully
			return true;
		}
		else
		{
			// The node was not found
			return false;
		}
	}


	// Find item x in the tree. Returns a pointer to the matching item
	// or NULL if not found -- O(log n)
	T* find(const T& x) const
	{
		tree_node* found = find(x, m_root);
		if (found == NULL) return NULL;
		return &found->element;
	}


	// Is the data present in the set? -- O(log n)
	bool contains(const T& x) const
	{
		if (find(x) != NULL)
			return true;
		else
			return false;
	}


	// Merge a different tree with ours -- O(n).
	bool merge(const simple_set<T>& b)
	{
		tree_node* c = b->clone();
		bool retVal = merge(c->m_root, m_root);

		// Re-balance the tree if the merge was successful
		if (retVal)
		{
			balanceTree();
		}
		else
		{
			global_free(c);
		}

		return retVal;
	}


	// Replace this set with another -- O(n)
	const simple_set& operator=(const simple_set& rhs)
	{
		// Don't clone if it's the same pointer
		if (this != &rhs)
		{
			clear();

			m_root = clone(rhs.m_root);
		}

		return *this;
	}


#ifdef SIMPLE_SET_DEBUG
	// Debug -- O(n log n)
	void printTree(std::ostream& out = std::cout) const
	{
		if(empty())
		{
			out << "Empty tree" << std::endl;
		}
		else
		{
			printTree(out, m_root);
		}
	}
#endif


private:
	// The AVL tree's root
	tree_node* m_root;

	// Find a node in the tree
	tree_node* findNode(const T& x) const
	{
		tree_node* node = find(x, m_root);
		if (node)
		{
			return node;
		}
		else
		{
			return NULL;
		}
	}


	// Insert item x into a subtree t (root) -- O(log n)
	bool insert(const T& x, tree_node*& t)
	{
		if (t == NULL)
		{
			t = global_alloc(tree_node(x, NULL, NULL, NULL));

			// An empty sub-tree here, insertion successful
			return true;
		}
		else if (x < t->element)
		{
			// O(log n)
			bool retVal = insert(x, t->left);

			if (retVal)
			{
				t->left->setParent(t);
				t->calcHeights();

				if(t->balanceFactor() < -1)
				{
					// See if it went left of the left
					if(x < t->left->element)
					{
						rotateWithLeftChild(t);
					}
					else
					{
						// The element goes on the right of the left
						doubleWithLeftChild(t);
					}
				}
			}

			return retVal;
		}
		else if (t->element < x)
		{
			bool retVal = insert(x, t->right);

			// Only do this if the insertion was successful
			if (retVal)
			{
				t->right->setParent(t);
				t->calcHeights();

				if (t->balanceFactor() > 1)
				{
					// See if it went right of the right
					if(t->right->element < x)
					{
						rotateWithRightChild(t);
					}
					else
					{
						// The element goes on the left of the right
						doubleWithRightChild(t);
					}
				}
			}

			return retVal;
		}
		else
		{
			return false;  // Duplicate
		}
	}


	// Recursively free all nodes in the tree -- O(n).
	void clearRecurse(tree_node*& t) const
	{
		if(t != NULL)
		{
			clearRecurse(t->left);
			clearRecurse(t->right);

			global_free(t);
		}
		t = NULL;
	}


	// Merge a tree with this one.  Private because external care is required.
	bool merge(tree_node* b, tree_node*& t)
	{
		if (!b)
		{
			return false;
		}
		else
		{
			bool retVal = false;

			if (t == NULL)
			{
				// Set this element to that subtree
				t = b;

				// The parent here should be NULL anyway, but we
				// set it just to be sure. This pointer will be
				// used as a flag to indicate where in the call
				// stack the tree was actually set.
				//
				// The middle layers of this method's call will
				// all have their parent references in tact since
				// no operations took place there.
				//
				//t->parent = NULL;
				t->setParent(NULL);

				// We were successful in merging
				retVal = true;
			}
			else if (b->element < t->element)
			{
				retVal = merge(b, t->left);

				// Only do this if the insertion actually took place
				if (retVal && !t->left->parent)
				{
					t->left->setParent(t);
					t->calcHeights();
				}
			}
			else if (t->element < b->element)
			{
				retVal = merge(b, t->right);

				// Only do this if the insertion was successful
				if (retVal && !t->right->parent)
				{
					t->right->setParent(t);
					t->calcHeights();
				}

				return retVal;
			}

			return retVal;
		}
	}


	// Find the smallest item's node in a subtree t -- O(log n).
	tree_node* findMin(tree_node* t) const
	{
		if(t == NULL)
		{
			return t;
		}

		while(t->left != NULL)
		{
			t = t->left;
		}

		return t;
	}


	// Find the smallest item's node in a subtree t -- O(log n).
	tree_node* findMax(tree_node* t) const
	{
		if(t == NULL)
		{
			return t;
		}

		while(t->right != NULL)
		{
			t = t->right;
		}

		return t;
	}


	// Find item x's node in subtree t -- O(log n)
	tree_node* find(const T& x, tree_node* t) const
	{
		while(t != NULL)
		{
			if (x < t->element)
			{
				t = t->left;
			}
			else if (t->element < x)
			{
				t = t->right;
			}
			else
			{
				return t;   // Match
			}
		}

		return NULL;   // No match
	}


	// Clone a subtree -- O(n)
	tree_node* clone(const tree_node* t) const
	{
		if(t == NULL)
		{
			return NULL;
		}
		else
		{
			// Create a node with the left and right nodes and a parent set to NULL
			tree_node* retVal = global_alloc(tree_node(t->element, NULL, clone(t->left), clone(t->right)));

			// Now set our children's parent node reference
			if (retVal->left) { retVal->left->setParent(retVal); }
			if (retVal->right) { retVal->right->setParent(retVal); }

			return retVal;
		}
	}


	// Rotate binary tree node with left child.
	// Single rotation for case 1 -- O(1).
	void rotateWithLeftChild(tree_node*& k2) const
	{
		tree_node* k1 = k2->left;
		tree_node* k2Parent = k2->parent;

		k2->setLeft(k1->right);
		if (k2->left) { k2->left->setParent(k2); }

		k1->setRight(k2);
		if (k1->right) { k1->right->setParent(k1); }

		k2 = k1;
		k2->setParent(k2Parent);

		k2->right->calcHeights();

	}


	// Rotate binary tree node with right child.
	// Single rotation for case 4 -- O(1).
	void rotateWithRightChild(tree_node*& k1) const
	{
		tree_node* k2 = k1->right;
		tree_node* k1Parent = k1->parent;

		k1->setRight(k2->left);
		if (k1->right) { k1->right->setParent(k1); }

		k2->setLeft(k1);
		if (k2->left) { k2->left->setParent(k2); }

		k1 = k2;
		k1->setParent(k1Parent);

		k1->left->calcHeights();

	}


	// Double rotate binary tree node: first left child
	// with its right child; then node k3 with new left child.
	// Double rotation for case 2 -- O(1).
	void doubleWithLeftChild(tree_node*& k3) const
	{
		rotateWithRightChild(k3->left);
		rotateWithLeftChild(k3);
	}


	// Double rotate binary tree node: first right child
	// with its left child; then node k1 with new right child.
	// Double rotation for case 3 -- O(1).
	void doubleWithRightChild(tree_node*& k1) const
	{
		rotateWithLeftChild(k1->right);
		rotateWithRightChild(k1);
	}


	// Removes a node. Returns true if the node was on the left side of its parent -- O(1).
	void removeNode(tree_node*& node)
	{
		// It is a leaf, simply remove the item and disconnect the parent
		if (node->isLeft())
		{
			node->parent->setLeft(NULL);
		}
		else // (node == node->parent->right)
		{
			if (node->parent) { node->parent->setRight(NULL); }
		}

		node->setParent(NULL);
	}


	// Swap one node with another -- O(1).
	void replaceNode(tree_node*& node1, tree_node*& node2)
	{
		// Save both parent references
		simple_set<T>* node1Parent = node1->parent;
		simple_set<T>* node2Parent = node2->parent;

		// First move node2 into node1's place
		if (node1Parent)
		{
			if (isLeft(node1))
			{
				node1Parent->setLeft(node2);
			}
			else // node1 is on the right
			{
				node1Parent->setRight(node2);
			}
		}
		node2->setParent(node1Parent);

		// Now move node1 into node2's place
		if (node2Parent)
		{
			if (isLeft(node2))
			{
				node2Parent->setLeft(node1);
			}
			else // node2 is on the right
			{
				node2Parent->setRight(node1);
			}
		}
		node1->setParent(node2Parent);
	}


	// Balances the tree starting at the root node
	void balanceTree() { balanceTree(m_root); }


	// Balance the tree starting at the given node -- O(n).
	void balanceTree(tree_node*& node)
	{
		if (node)
		{
			// First see what the balance factor for this node is
			int balFactor = node->balanceFactor();

			if (balFactor < -1)
			{
				// See if we're heavy left of the left
				if(node->left->balanceFactor() < 0)
				{
					rotateWithLeftChild(node);
				}
				else // if (node->left->balanceFactor() > 0)
				{
					// We're heavy on the right of the left
					doubleWithLeftChild(node);
				}
			}
			else if (balFactor > 1)
			{
				// See if it we're heavy right of the right
				if(node->right->balanceFactor() > 0)
				{
					rotateWithRightChild(node);
				}
				else // if (node->right->balanceFactor() < 0)
				{
					// The element goes on the left of the right
					doubleWithRightChild(node);
				}
			}
			else // if (balFactor >= -1 && balFactor <= 1)
			{
				// We're balanced here, but are our children balanced?
				balanceTree(node->left);
				balanceTree(node->right);
			}
		}
	}


	// Recursive helper function for public size()
	int sizeRecurse(const tree_node* currentNode) const
	{
		int nodeCount = 1;
		if (currentNode->left != NULL)
			nodeCount += sizeRecurse(currentNode->left);
		if (currentNode->right != NULL)
			nodeCount += sizeRecurse(currentNode->right);
		return nodeCount;
	}


#ifdef SIMPLE_SET_DEBUG
	// Debug.  Print from the start node, down -- O(n log n).
	void printTree(std::ostream& out, tree_node* t=NULL, int numTabs=0, char lr='_') const
	{
		if(t != NULL)
		{
			for (int i =0; i < numTabs; i++) { out << "  "; } out << "|_" << lr << "__ ";
			out << t->element << " {h = " << t->height() << ", b = " << t->balanceFactor() << "} ";
			// TODO: Reinstate out << std::hex << t << " (p = " << t->parent << ")" << std::dec;
			out << std::endl;

			printTree(out, t->left, numTabs + 1, '<');
			printTree(out, t->right, numTabs + 1, '>');
		}
	}
#endif
};


//
// ======================> avl_tree_node
// Member nodes of the simple_set's AVL tree
//

template <class T> class avl_tree_node
{
	friend class simple_set<T>;
	friend class simple_set_iterator<T>;
	typedef avl_tree_node<T> tree_node;

public:
	// Construction
	avl_tree_node(const T& theElement, avl_tree_node* p, avl_tree_node* lt, avl_tree_node* rt)
		: element(theElement),
		parent(p),
		left(lt),
		right(rt),
		m_height(1),
		m_balanceFactor(0)
	{ }


	// Are we to our parent's left?
	bool isLeft()
	{
		if (parent && this == parent->left)
		{
			return true;
		}
		else
		{
			return false;
		}
	}


	// Are we a leaf node?
	bool isLeaf() { return !left && !right; }


	// Set the parent pointer
	void setParent(tree_node* p)
	{
		// Set our new parent
		parent = p;
	}


	// Set the left child pointer
	void setLeft(tree_node* l)
	{
		// Set our new left node
		left = l;
	}


	// Set the right child pointer
	void setRight(tree_node* r)
	{
		// Set our new right node
		right = r;
	}


	// Recover the height
	int height() const
	{
		// The height is equal to the maximum of the right or left side's height plus 1
		// Trading memory for operation time can be done O(n) like this =>
		//  return max(left ? left->height() : 0, right ? right->height() : 0) + 1;
		return m_height;
	}


	// Recover the balance factor
	int balanceFactor() const
	{
		// The weight of a node is equal to the difference between
		// the weight of the left subtree and the weight of the
		// right subtree
		//
		// O(n) version =>
		//  return (right ? right->height() : 0) - (left ? left->height() : 0);
		//
		return m_balanceFactor;
	}


private:
	// Calculates all of the heights for this node and its ancestors -- O(log n).
	void calcHeights()
	{
		int rightHeight = (right ? right->m_height : 0);
		int leftHeight = (left ? left->m_height : 0);

		// Calculate our own height and balance factor -- O(1)
		m_height = maxInt(rightHeight, leftHeight) + 1;
		m_balanceFactor = (rightHeight - leftHeight);

		// And our parent's height and balance factor (and recurse) -- O(log n)
		if (parent)
		{
			parent->calcHeights();
		}
	}


	// Utility function - TODO replace
	int maxInt(const int& lhs, const int& rhs) const
	{
		return lhs > rhs ? lhs : rhs;
	}


private:
	T element;

	avl_tree_node* parent;
	avl_tree_node* left;
	avl_tree_node* right;

	int m_height;
	int m_balanceFactor;
};


//
// ======================> simple_set_iterator
// Iterator that allows for various set (AVL tree) navigation methods
// Points to elements of the set, rather than AVL tree nodes.
//
// PUBLIC OPERATIONS:
// current, first, last, next, count, indexof, byindex
//

template <class T>
class simple_set_iterator
{
	typedef avl_tree_node<T> tree_node;

public:
	enum TraversalType { PRE_ORDER, IN_ORDER, POST_ORDER, LEVEL_ORDER };

public:
	// construction
	simple_set_iterator(simple_set<T>& set, const TraversalType& tt=IN_ORDER)
		: m_set(&set),
			m_traversalType(tt),
			m_currentNode(NULL),
			m_endNode(NULL) { }

	~simple_set_iterator() { }


	// getters
	T* current() const { return m_currentNode; }


	// reset and return first item
	T* first()
	{
		m_currentNode = m_set->m_root;
		switch (m_traversalType)
		{
			case IN_ORDER:
			{
				// The current node is the smallest value
				m_currentNode = m_set->findMin(m_set->m_root);

				// The end case is the largest value
				m_endNode = m_set->findMax(m_set->m_root);

				return &m_currentNode->element;
			}

			default:
			{
				// TODO (better error message):
				printf("simple_set_iterator: Traversal type not yet supported.\n");
				return NULL;
			}
		}
		return NULL;
	}


	T* last()
	{
		return NULL;
	}


	// advance according to current state and traversal type
	T* next()
	{
		if (m_currentNode == NULL) return NULL;

		switch (m_traversalType)
		{
			case IN_ORDER:
			{
				// You are at the end
				if (m_currentNode == m_endNode)
					return NULL;

				if (m_currentNode->right != NULL)
				{
					// Gather the furthest left node of right subtree
					m_currentNode = m_currentNode->right;
					while (m_currentNode->left != NULL)
					{
						m_currentNode = m_currentNode->left;
					}
				}
				else
				{
					// No right subtree?  Move up the tree, looking for a left child link.
					tree_node* p = m_currentNode->parent;
					while (p != NULL && m_currentNode == p->right)
					{
						m_currentNode = p;
						p = p->parent;
					}
					m_currentNode = p;
				}

				return &m_currentNode->element;
			}

			default:
			{
				// TODO (better error message):
				printf("simple_set_iterator: Traversal type not yet supported.\n");
				return NULL;
			}
		}

		return NULL;
	}


	// return the number of items available
	int count()
	{
		return m_set->size();
	}


	// return the index of a given item in the virtual list
	// note: this function is destructive to any in-progress iterations!
	int indexof(T inData)
	{
		int index = 0;
		for (T* data = first(); data != last(); data = next(), index++)
			if (!(*data < inData) && !(inData < *data))
				return index;
		return -1;
	}


	// return the indexed item in the list
	// note: this function is destructive to any in-progress iterations!
	T* byindex(int index)
	{
		int count = 0;
		for (T* data = first(); data != last(); data = next(), count++)
			if (count == index)
				return data;
		return NULL;
	}


private:
	simple_set<T>* m_set;

	TraversalType m_traversalType;
	tree_node* m_currentNode;
	tree_node* m_endNode;
};

#endif
