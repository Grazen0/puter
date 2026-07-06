#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_array(size_t n, const int arr[static n])
{
    while (n--)
        printf("%d ", *arr++);
}

void print_array_u(size_t n, const unsigned arr[static n])
{
    while (n--)
        printf("%u ", *arr++);
}

void print_array_d(size_t n, const double arr[static n])
{
    while (n--)
        printf("%.2lf ", *arr++);
}

void swap(int *a, int *b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

size_t partition(size_t n, int arr[static n])
{
    size_t q = 0;

    int pivot = arr[n - 1];

    for (size_t i = 0; i < n; ++i) {
        if (arr[i] <= pivot)
            swap(&arr[i], &arr[q++]);
    }

    return q - 1;
}

void quicksort(size_t n, int arr[static n])
{
    if (n <= 1)
        return;

    size_t q = partition(n, arr);
    quicksort(q, arr);
    quicksort(n - q - 1, &arr[q + 1]);
}

void mergesort(size_t n, int arr[static n])
{
    if (n <= 1)
        return;

    size_t mid = n / 2;

    mergesort(mid, arr);
    mergesort(n - mid, &arr[mid]);

    size_t l = 0;
    size_t r = mid;

    int out[n];

    for (size_t i = 0; i < n; ++i) {
        if (r >= n || (l < mid && arr[l] < arr[r]))
            out[i] = arr[l++];
        else
            out[i] = arr[r++];
    }

    memcpy(arr, out, n * sizeof(*arr));
}

void max_heapify(size_t n, int arr[static n], size_t idx)
{
    if (idx >= n)
        return;

    while (true) {
        size_t l = (2 * idx) + 1;
        size_t r = (2 * idx) + 2;
        size_t largest = idx;

        if (l < n && arr[l] > arr[largest])
            largest = l;

        if (r < n && arr[r] > arr[largest])
            largest = r;

        if (largest == idx)
            break;

        swap(&arr[idx], &arr[largest]);
        idx = largest;
    }
}

void heapsort(size_t n, int arr[static n])
{
    for (size_t i = 0; i <= n / 2; ++i)
        max_heapify(n, arr, (n / 2) - i);

    while (n > 0) {
        swap(&arr[0], &arr[--n]);
        max_heapify(n, arr, 0);
    }
}

typedef void ExploreFn(void *arg, size_t n, int arr[]);

void permutate_inner(size_t n, int arr[], size_t s, ExploreFn *fn, void *fn_arg)
{
    assert(s <= n);

    if (s == n) {
        fn(fn_arg, n, arr);
        return;
    }

    for (size_t i = s; i < n; ++i) {
        swap(&arr[s], &arr[i]);
        permutate_inner(n, arr, s + 1, fn, fn_arg);
        swap(&arr[s], &arr[i]);
    }
}

void permutate(size_t n, int arr[static n], ExploreFn *fn, void *fn_arg)
{
    permutate_inner(n, arr, 0, fn, fn_arg);
}

void explore(void *arg, size_t n, int arr[static n])
{
    print_array(n, arr);
    printf("\n");

    int *count = arg;
    ++(*count);
}

long p(int n)
{
    long dp[n + 1][n + 1];

    for (int i = 0; i <= n; ++i) {
        for (int j = 0; j <= n; ++j) {
            long sum = 0;

            if (i <= 1) {
                sum = 1;
            } else {
                for (int k = 1; k <= j; ++k) {
                    if (k <= i)
                        sum += dp[i - k][k];
                }
            }

            dp[i][j] = sum;
        }
    }

    return dp[n][n];
}

long lmax(long a, long b)
{
    return a > b ? a : b;
}

long lmin(long a, long b)
{
    return a < b ? a : b;
}

long opt_inner(size_t n, int a[static n], size_t k, int ps[static n + 1])
{
    if (k == 1)
        return ps[n];

    long ans = LONG_MAX;

    for (size_t i = k; i < n; ++i) {
        long sub_opt = lmax(ps[n] - ps[i], opt_inner(i, a, k - 1, ps));
        ans = lmin(ans, sub_opt);
    }

    return ans;
}

long opt(size_t n, int a[static n], size_t k)
{
    int ps[n + 1];
    ps[0] = 0;

    for (size_t i = 0; i < n; ++i)
        ps[i + 1] = ps[i] + a[i];

    return opt_inner(n, a, k, ps);
}

typedef struct TreapNode TreapNode;
struct TreapNode {
    int data;
    int priority;
    TreapNode *left;
    TreapNode *right;
};

void tnode_print(const TreapNode *node, const size_t depth)
{
    if (node == NULL)
        return;

    printf("%d (%zu)\n", node->data, depth);
    tnode_print(node->left, depth + 1);
    tnode_print(node->right, depth + 1);
}

#define STACK_MAX 1000

void tnode_insert(TreapNode **root, int data)
{
    TreapNode **cur = root;

    TreapNode **stack[STACK_MAX];
    size_t stack_len = 0;

    while (*cur != NULL) {
        if ((*cur)->data == data)
            return;

        stack[stack_len++] = cur;

        if (data < (*cur)->data)
            cur = &(*cur)->left;
        else
            cur = &(*cur)->right;
    }

    *cur = calloc(1, sizeof(**cur));
    **cur = (TreapNode){
        .data = data,
        .priority = rand(),
        .left = NULL,
        .right = NULL,
    };

    TreapNode *inserted = *cur;

    while (1) {
        TreapNode **parent_holder = stack[--stack_len];
        TreapNode *parent = *parent_holder;

        if (parent->priority > inserted->priority) {
        }
    }
}

typedef unsigned DigitExtractor(void *arg, unsigned n);

void counting_sort(size_t n, unsigned arr[static n], size_t k,
                   DigitExtractor *ex, void *ex_arg)
{
    unsigned arr_ex[n];
    unsigned b[n];
    size_t c[k];

    for (size_t i = 0; i < k; ++i)
        c[i] = 0;

    for (size_t j = 0; j < n; ++j) {
        arr_ex[j] = ex(ex_arg, arr[j]);
        ++c[arr_ex[j]];
    }

    for (unsigned i = 1; i < k; ++i)
        c[i] += c[i - 1];

    for (size_t j = n; j > 0; --j) {
        b[--c[arr_ex[j - 1]]] = arr[j - 1];
    }

    memcpy(arr, b, n * sizeof(*arr));
}

typedef struct {
    unsigned base;
    unsigned digit;
} DigitInfo;

unsigned get_nth_digit(void *arg, unsigned n)
{
    DigitInfo *info = arg;
    return (n / (unsigned)pow(info->base, info->digit)) % info->base;
}

void radix_sort(size_t n, unsigned arr[static n], size_t base, unsigned d)
{
    DigitInfo info = {.base = base};

    for (info.digit = 0; info.digit < d; ++info.digit)
        counting_sort(n, arr, base, get_nth_digit, &info);
}

typedef struct ListNode ListNode;
struct ListNode {
    double value;
    ListNode *next;
};

void list_sort(ListNode **root)
{
    ListNode *cur = *root;
    *root = NULL;

    while (cur != NULL) {
        ListNode **insert_cur = root;

        while (*insert_cur != NULL && cur->value >= (*insert_cur)->value)
            insert_cur = &(*insert_cur)->next;

        ListNode *next = cur->next;

        cur->next = *insert_cur;
        *insert_cur = cur;

        cur = next;
    }
}

void bucket_sort(size_t n, double arr[static n])
{
    ListNode *buckets[n];

    for (size_t i = 0; i < n; ++i)
        buckets[i] = NULL;

    for (size_t i = 0; i < n; ++i) {
        size_t b = (size_t)((double)n * arr[i]);

        ListNode *new_node = calloc(1, sizeof(*new_node));
        assert(new_node != NULL);

        *new_node = (ListNode){
            .value = arr[i],
            .next = buckets[b],
        };

        buckets[b] = new_node;
    }

    size_t idx = 0;

    for (size_t i = 0; i < n; ++i) {
        list_sort(&buckets[i]);

        ListNode *cur = buckets[i];

        while (cur != NULL) {
            arr[idx++] = cur->value;
            cur = cur->next;
        }
    }
}

void shuffle(size_t n, int arr[static n])
{
    for (size_t i = 0; i < n; ++i) {
        size_t j = i + (rand() % (n - i));

        if (i != j)
            swap(&arr[i], &arr[j]);
    }
}

bool is_sorted(size_t n, const int arr[static n])
{
    for (size_t i = 0; i < n - 1; ++i) {
        if (arr[i] > arr[i + 1])
            return false;
    }

    return true;
}

void bogosort(size_t n, int arr[static n])
{
    while (true) {
        if (is_sorted(n, arr))
            return;

        shuffle(n, arr);
    }
}

int main()
{
    return 0;
}
