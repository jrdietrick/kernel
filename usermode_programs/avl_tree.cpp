#include "userlib"
#include "avl_tree.h"


Node::Node (
    int value
    ) : left_(nullptr), right_(nullptr),
        value_(value), height_(1), count_(1)
{
}

void Node::updateHeightAndCount (
    )
{
    int left_height = left_ ? left_->height_ : 0;
    int right_height = right_ ? right_->height_ : 0;
    height_ = MAX(left_height, right_height) + 1;

    int left_count = left_ ? left_->count_ : 0;
    int right_count = right_ ? right_->count_ : 0;
    count_ = left_count + right_count + 1;
}

int Node::balanceFactor (
    )
{
    int left_height = left_ ? left_->height_ : 0;
    int right_height = right_ ? right_->height_ : 0;
    return left_height - right_height;
}

Node* Node::rebalance (
    )
{
    updateHeightAndCount();

    int parent_balance_factor = balanceFactor();
    int child_balance_factor;
    Node* new_root = this;

    assert(parent_balance_factor >= -2 && parent_balance_factor <= 2);

    if (parent_balance_factor >= -1 && parent_balance_factor <= 1) {
        // Nothing to do, just return the root
        return new_root;
    }

    if (parent_balance_factor == -2) {
        // Tree is right-heavy
        child_balance_factor = right_->balanceFactor();
        if (child_balance_factor == 1) {
            // Rotate right through child
            right_ = right_->rotateRight();
        }
        // Rotate left through us
        new_root = rotateLeft();
    } else {
        // Tree is left-heavy
        child_balance_factor = left_->balanceFactor();
        if (child_balance_factor == -1) {
            // Rotate left through child
            left_ = left_->rotateLeft();
        }
        // Rotate right through us
        new_root = rotateRight();
    }

    return new_root;
}

Node* Node::insert (
    Node* node
    )
{
    // We don't allow duplicate values right now
    assert(node->value_ != value_);

    if (node->value_ < value_) {
        // Go left
        if (left_) {
            left_ = left_->insert(node);
        } else {
            left_ = node;
        }
        return rebalance();
    }

    // Go right
    if (right_) {
        right_ = right_->insert(node);
    } else {
        right_ = node;
    }
    return rebalance();
}

Node* Node::find (
    int value
    )
{
    if (value < value_) {
        if (!left_) {
            return nullptr;
        }
        return left_->find(value);
    }

    if (value > value_) {
        if (!right_) {
            return nullptr;
        }
        return right_->find(value);
    }

    return this;
}

Node* Node::extract (
    int value,
    Node** extracted
    )
{
    if (value < value_) {
        // Go left
        if (left_) {
            left_ = left_->extract(value, extracted);
        } else {
            *extracted = nullptr;
        }
        return rebalance();
    }

    if (value > value_) {
        // Go right
        if (right_) {
            right_ = right_->extract(value, extracted);
        } else {
            *extracted = nullptr;
        }
        return rebalance();
    }

    // We are the node to remove. Swap ourselves
    // with the in-order predecessor if there is
    // one, else just return the right subtree
    if (!left_) {
        Node* new_root = right_;
        *extracted = this;
        left_ = nullptr;
        right_ = nullptr;
        updateHeightAndCount();
        return new_root;
    }

    // Find the in-order predecessor and swap
    Node* predecessor;
    left_ = left_->extractMaximum(&predecessor);
    predecessor->left_ = left_;
    predecessor->right_ = right_;
    predecessor->updateHeightAndCount();

    *extracted = this;
    left_ = nullptr;
    right_ = nullptr;
    updateHeightAndCount();

    return predecessor->rebalance();
}

int Node::getValue (
    )
{
    return value_;
}

Node* Node::rotateLeft (
    )
{
    Node* new_root = right_;
    right_ = new_root->left_;
    new_root->left_ = this;

    // Update height and count from bottom up,
    // to fix the fact that we (old root) have
    // gone down a level
    updateHeightAndCount();
    new_root->updateHeightAndCount();

    return new_root;
}

Node* Node::rotateRight (
    )
{
    Node* new_root = left_;
    left_ = new_root->right_;
    new_root->right_ = this;

    // Update height and count from bottom up
    updateHeightAndCount();
    new_root->updateHeightAndCount();

    return new_root;
}

Node* Node::extractMaximum (
    Node** extracted
    )
{
    if (!right_) {
        // We are the max; pull ourselves out
        *extracted = this;
        return left_;
    }
    right_ = right_->extractMaximum(extracted);
    return rebalance();
}

void printNodeShortcut (
    Node* node
    )
{
    printf("visited %d\n", node->getValue());
}

void Node::inOrderTraversal (
    )
{
    if (left_) {
        left_->inOrderTraversal();
    }
    printNodeShortcut(this);
    if (right_) {
        right_->inOrderTraversal();
    }
}

void Node::preOrderTraversal (
    )
{
    printNodeShortcut(this);
    if (left_) {
        left_->preOrderTraversal();
    }
    if (right_) {
        right_->preOrderTraversal();
    }
}

void Node::postOrderTraversal (
    )
{
    if (left_) {
        left_->postOrderTraversal();
    }
    if (right_) {
        right_->postOrderTraversal();
    }
    printNodeShortcut(this);
}

int main (
    )
{
    Node* root = new Node(0);
    for (int i = 1; i < 10; i++) {
        root = root->insert(new Node(i));
    }

    // Nodes 0 through 9 should be found
    for (int i = 0; i < 10; i++) {
        assert(root->find(i) != nullptr);
    }

    // 10 - 19 should not be
    for (int i = 10; i < 20; i++) {
        assert(root->find(i) == nullptr);
    }

    root->inOrderTraversal();

    // Remove 0 through 4
    for (int i = 0; i < 5; i++) {
        Node* extracted;
        root = root->extract(i, &extracted);
        delete extracted;
    }

    // ... and they should not be found
    for (int i = 0; i < 5; i++) {
        assert(root->find(i) == nullptr);
    }

    // Free the remaining nodes
    for (int i = 5; i < 10; i++) {
        Node* extracted;
        root = root->extract(i, &extracted);
        delete extracted;
    }

    // Should be nothing left
    assert(root == nullptr);

    return 0;
}

extern "C" void _start (
    )
{
    _exit(main());
}