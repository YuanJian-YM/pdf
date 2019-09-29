#include "pdf_cmap.h"
namespace pdf
{
int hex_char_to_num(char sCh)
{
    char dCh = 0;
    if (sCh >= '0' && sCh <= '9')
    {
        dCh = sCh - '0';
    }
    else if (sCh >= 'A' && sCh <= 'F')
    {
        dCh = (sCh - 'A') + 10;
    }
    else if (sCh >= 'a' && sCh <= 'f')
    {
        dCh = (sCh - 'a') + 10;
    }
    else
    {
        cout << "incorrect hex char!" << endl;
    }
}
void getArray(const string &arrstr, vector<string> &vec)
{
    string value = "";
    int count = (int)arrstr.size();

    if (count < 2 || arrstr[0] != '[' || arrstr[count - 1] != ']')
    {
        cout << "incorrect array str!" << endl;
        return;
    }

    for (int i = 1; i < count - 1; ++i)
    {
        if (arrstr[i] == ' ')
        {
            if (!value.empty())
            {
                vec.push_back(value);
                value = "";
            }
        }
        else
        {
            value.push_back(arrstr[i]);
        }
    }
    if (!value.empty())
    {
        vec.push_back(value);
    }
}
void appendString(const string &oriStr, string &destStr)
{
    if (!oriStr.empty())
    {
        char first = oriStr.front();
        char back = oriStr.back();

        if (first == '(' && back == ')')
        {
            destStr += oriStr.substr(1, oriStr.size() - 2);
        }
        else if (first == '<' && back == '>')
        {
            bool needAdd = true;
            if (oriStr.size() % 2 == 0)
            {
                needAdd = false;
            }
            for (int i = 1; i < (int)oriStr.size() - (needAdd ? 2 : 1); i += 2)
            {
                char ch = 0;
                ch += hex_char_to_num(oriStr[i]) * 16;
                ch += hex_char_to_num(oriStr[i + 1]);
                destStr.push_back(ch);
            }
            if (needAdd)
            {
                char ch = 0;
                ch += hex_char_to_num(oriStr[oriStr.size() - 2]) * 16;
                destStr.push_back(ch);
            }
        }
        else if (first == '[' && back == ']')
        {
            stack<char> bracket;
            string temp = "";
            for (int i = 1; i < (int)oriStr.size() - 1; ++i)
            {
                if (oriStr[i] == '<' || oriStr[i] == '(' || oriStr[i] == '[')
                {
                    bracket.push(oriStr[i]);
                    temp.push_back(oriStr[i]);
                }
                else if (oriStr[i] == '>' || oriStr[i] == ')' || oriStr[i] == ']')
                {
                    bracket.pop();
                    temp.push_back(oriStr[i]);
                    appendString(temp, destStr);
                    temp = "";
                }
                else if (!bracket.empty())
                {
                    temp.push_back(oriStr[i]);
                }
                else
                {
                    /* code */
                }
            }
        }
        else
        {
            cout << "incorrect string str!" << endl;
        }
    }
}

PDFCmap::PDFCmap() : cmap_name(""),
                     usecmap_name(""),
                     usecmap(nullptr),
                     wmode(0),
                     codespace_len(0),
                     rlen(0),
                     rcap(0),
                     xlen(0),
                     xcap(0),
                     mlen(0),
                     mcap(0),
                     dlen(0),
                     dcap(0),
                     tlen(0),
                     tcap(0),
                     ttop(0),
                     dict(nullptr),
                     ranges(nullptr),
                     xranges(nullptr),
                     mranges(nullptr),
                     tree(nullptr)
{
    memset(codespace, 0, sizeof(codespace));
}

PDFCmap::~PDFCmap()
{
    if (usecmap)
        delete usecmap;
    if (dict)
        delete dict;
    if (ranges)
        delete[] ranges;
    if (xranges)
        delete[] xranges;
    if (mranges)
        delete[] mranges;
    if (tree)
        delete tree;
}

bool finishWord(char ch, string &word, stack<char> &backet, int &type, int change)
{
    if (backet.empty())
    {
        if (!word.empty())
        {
            return true;
        }
        type = change;
    }
    word.push_back(ch);

    return false;
}

char *nextWord(char *p, pair<int, string> &next)
{
    int type = WORD_OTHER;
    string word = "";
    stack<char> backet;

    next.first = type;
    next.second = word;
    if (p && *p != '\0')
    {
        while (*p != '\0')
        {
            if (*p == '/')
            {
                if (backet.empty())
                {
                    if (!word.empty())
                    {
                        break;
                    }
                    type = WORD_NAME;
                }
                else
                {
                    word.push_back(*p);
                }
            }
            else if (*p == ' ')
            {
                if (backet.empty())
                {
                    if (!word.empty())
                    {
                        break;
                    }
                }
                else
                {
                    word.push_back(*p);
                }
            }
            else if (*p == '\n')
            {
                if (backet.empty())
                {
                    if (!word.empty())
                    {
                        break;
                    }
                }
                else
                {
                    char ch = *(p + 1);
                    if (ch != '/' && ch != '>' && ch != ')' && ch != ']')
                    {
                        word.push_back(' ');
                    }
                }
            }
            else if (*p == '(')
            {
                if (finishWord(*p, word, backet, type, WORD_STRING))
                {
                    break;
                }
                backet.push(*p);
            }
            else if (*p == '<')
            {
                int tType = *(p - 1) != '<' && *(p + 1) == '<' ? WORD_DICT : WORD_HEX_STRING;
                if (finishWord(*p, word, backet, type, tType))
                {
                    break;
                }
                backet.push(*p);
            }
            else if (*p == '[')
            {
                if (finishWord(*p, word, backet, type, WORD_ARRAY))
                {
                    break;
                }
                backet.push(*p);
            }
            else if (*p == ')' || *p == '>' || *p == ']')
            {
                word.push_back(*p);
                backet.pop();
            }
            else
            {
                word.push_back(*p);
            }
            p++;
        }
    }
    else
    {
        type = WORD_ERROR;
        p = nullptr;
    }

    if (type == WORD_HEX_STRING || type == WORD_STRING)
    {
        next.first = WORD_STRING;
        appendString(word, next.second);
    }
    else
    {
        next.first = type;
        next.second = word;
    }

    return p;
}

void walk_splay(cmap_splay *tree, unsigned int node, void (*fn)(cmap_splay *, void *), void *arg)
{
    int from = 0;

    while (node != EMPTY)
    {
        switch (from)
        {
        case 0:
            if (tree[node].left != EMPTY)
            {
                node = tree[node].left;
                from = 0;
                break;
            }
            /* fallthrough */
        case 1:
            fn(&tree[node], arg);
            if (tree[node].right != EMPTY)
            {
                node = tree[node].right;
                from = 0;
                break;
            }
            /* fallthrough */
        case 2:
        {
            unsigned int parent = tree[node].parent;
            if (parent == EMPTY)
                return;
            if (tree[parent].left == node)
                from = 1;
            else
            {
                assert(tree[parent].right == node);
                from = 2;
            }
            node = parent;
        }
        }
    }
}

void count_node_types(cmap_splay *node, void *arg)
{
    int *counts = (int *)arg;

    if (node->many)
        counts[2]++;
    else if (node->low <= 0xffff && node->high <= 0xFFFF && node->out <= 0xFFFF)
        counts[0]++;
    else
        counts[1]++;
}

void copy_node_types(cmap_splay *node, void *arg)
{
    PDFCmap *cmap = (PDFCmap *)arg;

    if (node->many)
    {
        cmap->mranges[cmap->mlen].low = node->low;
        cmap->mranges[cmap->mlen].out = node->out;
        cmap->mlen++;
    }
    else if (node->low <= 0xffff && node->high <= 0xFFFF && node->out <= 0xFFFF)
    {
        cmap->ranges[cmap->rlen].low = node->low;
        cmap->ranges[cmap->rlen].high = node->high;
        cmap->ranges[cmap->rlen].out = node->out;
        cmap->rlen++;
    }
    else
    {
        cmap->xranges[cmap->xlen].low = node->low;
        cmap->xranges[cmap->xlen].high = node->high;
        cmap->xranges[cmap->xlen].out = node->out;
        cmap->xlen++;
    }
}

void sortCmap(PDFCmapPtr cmap)
{
    int counts[3];

    if (cmap->tree == NULL)
        return;

    counts[0] = 0;
    counts[1] = 0;
    counts[2] = 0;

    walk_splay(cmap->tree, cmap->ttop, count_node_types, &counts);

    cmap->ranges = new cmap_range[counts[0] * sizeof(*cmap->ranges)];
    cmap->rcap = counts[0];
    cmap->xranges = new cmap_xrange[counts[1] * sizeof(*cmap->xranges)];
    cmap->xcap = counts[1];
    cmap->mranges = new cmap_mrange[counts[2] * sizeof(*cmap->mranges)];
    cmap->mcap = counts[2];

    walk_splay(cmap->tree, cmap->ttop, copy_node_types, &(*cmap));

    delete cmap->tree;
    cmap->tree = NULL;
}

void addCodeSpace(PDFCmapPtr cmap, int low, int high, int n)
{
    cmap->codespace[cmap->codespace_len].n = n;
    cmap->codespace[cmap->codespace_len].low = low;
    cmap->codespace[cmap->codespace_len].high = high;
    cmap->codespace_len++;
}

int codeFromString(string &buf)
{
    unsigned int a = 0;
    int start = 0;
    int end = (int)buf.size();
    for (int i = start; i < end; ++i)
    {
        a = (a << 8) | (unsigned char)(buf[i]);
    }

    return a;
}

char *parseCodespaceRange(PDFCmapPtr cmap, char *p)
{
    while (p)
    {
        pair<int, string> next;
        p = nextWord(p, next);
        if (next.first == WORD_ERROR || next.second == "endcodespacerange")
        {
            return p;
        }
        else if (next.first == WORD_STRING)
        {
            int lo = codeFromString(next.second);
            p = nextWord(p, next);
            if (next.first == WORD_STRING)
            {
                int hi = codeFromString(next.second);
                addCodeSpace(cmap, lo, hi, next.second.size());
            }
        }
    }

    return p;
}

void moveToRoot(cmap_splay *tree, unsigned int x)
{
    if (x == EMPTY)
        return;
    do
    {
        unsigned int z, zp;
        unsigned int y = tree[x].parent;
        if (y == EMPTY)
            break;
        z = tree[y].parent;
        if (z == EMPTY)
        {
            /* Case 3 */
            tree[x].parent = EMPTY;
            tree[y].parent = x;
            if (tree[y].left == x)
            {
                /* Case 3 */
                tree[y].left = tree[x].right;
                if (tree[y].left != EMPTY)
                    tree[tree[y].left].parent = y;
                tree[x].right = y;
            }
            else
            {
                /* Case 3 - reflected */
                assert(tree[y].right == x);
                tree[y].right = tree[x].left;
                if (tree[y].right != EMPTY)
                    tree[tree[y].right].parent = y;
                tree[x].left = y;
            }
            break;
        }

        zp = tree[z].parent;
        tree[x].parent = zp;
        if (zp != EMPTY)
        {
            if (tree[zp].left == z)
                tree[zp].left = x;
            else
            {
                assert(tree[zp].right == z);
                tree[zp].right = x;
            }
        }
        tree[y].parent = x;
        if (tree[y].left == x)
        {
            tree[y].left = tree[x].right;
            if (tree[y].left != EMPTY)
                tree[tree[y].left].parent = y;
            tree[x].right = y;
            if (tree[z].left == y)
            {
                /* Case 1 */
                tree[z].parent = y;
                tree[z].left = tree[y].right;
                if (tree[z].left != EMPTY)
                    tree[tree[z].left].parent = z;
                tree[y].right = z;
            }
            else
            {
                /* Case 2 - reflected */
                assert(tree[z].right == y);
                tree[z].parent = x;
                tree[z].right = tree[x].left;
                if (tree[z].right != EMPTY)
                    tree[tree[z].right].parent = z;
                tree[x].left = z;
            }
        }
        else
        {
            assert(tree[y].right == x);
            tree[y].right = tree[x].left;
            if (tree[y].right != EMPTY)
                tree[tree[y].right].parent = y;
            tree[x].left = y;
            if (tree[z].left == y)
            {
                /* Case 2 */
                tree[z].parent = x;
                tree[z].left = tree[x].right;
                if (tree[z].left != EMPTY)
                    tree[tree[z].left].parent = z;
                tree[x].right = z;
            }
            else
            {
                /* Case 1 - reflected */
                assert(tree[z].right == y);
                tree[z].parent = y;
                tree[z].right = tree[y].left;
                if (tree[z].right != EMPTY)
                    tree[tree[z].right].parent = z;
                tree[y].left = z;
            }
        }
    } while (1);
}

unsigned int deleteNode(PDFCmapPtr cmap, unsigned int current)
{
    cmap_splay *tree = cmap->tree;
    unsigned int parent;
    unsigned int replacement;

    assert(current != EMPTY);

    parent = tree[current].parent;
    if (tree[current].right == EMPTY)
    {
        if (parent == EMPTY)
        {
            replacement = cmap->ttop = tree[current].left;
        }
        else if (tree[parent].left == current)
        {
            replacement = tree[parent].left = tree[current].left;
        }
        else
        {
            assert(tree[parent].right == current);
            replacement = tree[parent].right = tree[current].left;
        }
        if (replacement != EMPTY)
            tree[replacement].parent = parent;
        else
            replacement = parent;
    }
    else if (tree[current].left == EMPTY)
    {
        if (parent == EMPTY)
        {
            replacement = cmap->ttop = tree[current].right;
        }
        else if (tree[parent].left == current)
        {
            replacement = tree[parent].left = tree[current].right;
        }
        else
        {
            assert(tree[parent].right == current);
            replacement = tree[parent].right = tree[current].right;
        }
        if (replacement != EMPTY)
            tree[replacement].parent = parent;
        else
            replacement = parent;
    }
    else
    {
        /* Hard case, find the in-order predecessor of current */
        int amputee = current;
        replacement = tree[current].left;
        while (tree[replacement].right != EMPTY)
        {
            amputee = replacement;
            replacement = tree[replacement].right;
        }
        /* Remove replacement from the tree */
        if (amputee == current)
        {
            tree[amputee].left = tree[replacement].left;
            if (tree[amputee].left != EMPTY)
                tree[tree[amputee].left].parent = amputee;
        }
        else
        {
            tree[amputee].right = tree[replacement].left;
            if (tree[amputee].right != EMPTY)
                tree[tree[amputee].right].parent = amputee;
        }
        /* Insert replacement in place of current */
        tree[replacement].parent = parent;
        if (parent == EMPTY)
        {
            tree[replacement].parent = EMPTY;
            cmap->ttop = replacement;
        }
        else if (tree[parent].left == current)
            tree[parent].left = replacement;
        else
        {
            assert(tree[parent].right == current);
            tree[parent].right = replacement;
        }
        tree[replacement].left = tree[current].left;
        if (tree[replacement].left != EMPTY)
            tree[tree[replacement].left].parent = replacement;
        tree[replacement].right = tree[current].right;
        if (tree[replacement].right != EMPTY)
            tree[tree[replacement].right].parent = replacement;
    }

    /* current is now unlinked. We need to remove it from our array. */
    cmap->tlen--;
    if (current != cmap->tlen)
    {
        if (replacement == cmap->tlen)
            replacement = current;
        tree[current] = tree[cmap->tlen];
        parent = tree[current].parent;
        if (parent == EMPTY)
            cmap->ttop = current;
        else if (tree[parent].left == cmap->tlen)
            tree[parent].left = current;
        else
        {
            assert(tree[parent].right == cmap->tlen);
            tree[parent].right = current;
        }
        if (tree[current].left != EMPTY)
        {
            assert(tree[tree[current].left].parent == cmap->tlen);
            tree[tree[current].left].parent = current;
        }
        if (tree[current].right != EMPTY)
        {
            assert(tree[tree[current].right].parent == cmap->tlen);
            tree[tree[current].right].parent = current;
        }
    }

    /* Return the node that we should continue searching from */
    return replacement;
}

void addRange(PDFCmapPtr cmap, unsigned int low, unsigned int high, unsigned int out, int check_for_overlap, int many)
{
    int current;
    cmap_splay *tree;
    int i;
    int inrange = 0;
    unsigned int k, count;

    if (low > high)
    {
        return;
    }

    count = high - low + 1;
    for (k = 0; k < count; k++)
    {
        unsigned int c = low + k;

        inrange = 0;
        for (i = 0; i < cmap->codespace_len; i++)
        {
            if (cmap->codespace[i].low <= c && c <= cmap->codespace[i].high)
                inrange = 1;
        }
        if (!inrange)
        {
            return;
        }
    }

    tree = cmap->tree;

    if (cmap->tlen)
    {
        unsigned int move = cmap->ttop;
        unsigned int gt = EMPTY;
        unsigned int lt = EMPTY;
        if (check_for_overlap)
        {
            /* Check for collision with the current node */
            do
            {
                current = move;
                /* Cases we might meet:
                    * tree[i]:        <----->
                    * case 0:     <->
                    * case 1:     <------->
                    * case 2:     <------------->
                    * case 3:           <->
                    * case 4:           <------->
                    * case 5:                 <->
                    */
                if (low <= tree[current].low && tree[current].low <= high)
                {
                    /* case 1, reduces to case 0 */
                    /* or case 2, deleting the node */
                    tree[current].out += high + 1 - tree[current].low;
                    tree[current].low = high + 1;
                    if (tree[current].low > tree[current].high)
                    {
                        /* update lt/gt references that will be moved/stale after deleting current */
                        if (gt == cmap->tlen - 1)
                            gt = current;
                        if (lt == cmap->tlen - 1)
                            lt = current;
                        /* delete_node() moves the element at cmap->tlen-1 into current */
                        move = deleteNode(cmap, current);
                        current = EMPTY;
                        continue;
                    }
                }
                else if (low <= tree[current].high && tree[current].high <= high)
                {
                    /* case 4, reduces to case 5 */
                    tree[current].high = low - 1;
                    assert(tree[current].low <= tree[current].high);
                }
                else if (tree[current].low < low && high < tree[current].high)
                {
                    /* case 3, reduces to case 5 */
                    int new_high = tree[current].high;
                    tree[current].high = low - 1;
                    addRange(cmap, high + 1, new_high, tree[current].out + high + 1 - tree[current].low, 0, tree[current].many);
                    tree = cmap->tree;
                }
                /* Now look for where to move to next (left for case 0, right for case 5) */
                if (tree[current].low > high)
                {
                    move = tree[current].left;
                    gt = current;
                }
                else
                {
                    move = tree[current].right;
                    lt = current;
                }
            } while (move != EMPTY);
        }
        else
        {
            do
            {
                current = move;
                if (tree[current].low > high)
                {
                    move = tree[current].left;
                    gt = current;
                }
                else
                {
                    move = tree[current].right;
                    lt = current;
                }
            } while (move != EMPTY);
        }
        /* current is now the node to which we would be adding the new node */
        /* lt is the last node we traversed which is lt the new node. */
        /* gt is the last node we traversed which is gt the new node. */

        if (!many)
        {
            /* Check for the 'merge' cases. */
            if (lt != EMPTY && !tree[lt].many && tree[lt].high == low - 1 && tree[lt].out - tree[lt].low == out - low)
            {
                tree[lt].high = high;
                if (gt != EMPTY && !tree[gt].many && tree[gt].low == high + 1 && tree[gt].out - tree[gt].low == out - low)
                {
                    tree[lt].high = tree[gt].high;
                    deleteNode(cmap, gt);
                }
                return;
            }
            if (gt != EMPTY && !tree[gt].many && tree[gt].low == high + 1 && tree[gt].out - tree[gt].low == out - low)
            {
                tree[gt].low = low;
                tree[gt].out = out;
                return;
            }
        }
    }
    else
        current = EMPTY;

    if (cmap->tlen == cmap->tcap)
    {
        int new_cap = cmap->tcap ? cmap->tcap * 2 : 256;
        tree = cmap->tree = new cmap_splay[new_cap * sizeof(*cmap->tree)];
        cmap->tcap = new_cap;
    }
    tree[cmap->tlen].low = low;
    tree[cmap->tlen].high = high;
    tree[cmap->tlen].out = out;
    tree[cmap->tlen].parent = current;
    tree[cmap->tlen].left = EMPTY;
    tree[cmap->tlen].right = EMPTY;
    tree[cmap->tlen].many = many;
    cmap->tlen++;
    if (current == EMPTY)
        cmap->ttop = 0;
    else if (tree[current].low > high)
        tree[current].left = cmap->tlen - 1;
    else
    {
        assert(tree[current].high < low);
        tree[current].right = cmap->tlen - 1;
    }
    moveToRoot(tree, cmap->tlen - 1);
    cmap->ttop = cmap->tlen - 1;
}

void addMrange(PDFCmapPtr cmap, unsigned int low, int *out, int len)
{
    int out_pos;

    if (cmap->dlen + len + 1 > cmap->dcap)
    {
        int new_cap = cmap->dcap ? cmap->dcap * 2 : 256;
        cmap->dict = new int[new_cap * sizeof(*cmap->dict)];
        cmap->dcap = new_cap;
    }
    out_pos = cmap->dlen;
    cmap->dict[out_pos] = len;
    memcpy(&cmap->dict[out_pos + 1], out, sizeof(int) * len);
    cmap->dlen += len + 1;

    addRange(cmap, low, low, out_pos, 1, 1);
}

void mapOneToMany(PDFCmapPtr cmap, unsigned int low, int *values, int len)
{
    if (len == 1)
    {
        addRange(cmap, low, low, values[0], 1, 0);
        return;
    }
    if (len == 2 &&
        values[0] >= 0xD800 && values[0] <= 0xDBFF &&
        values[1] >= 0xDC00 && values[1] <= 0xDFFF)
    {
        int rune = ((values[0] - 0xD800) << 10) + (values[1] - 0xDC00) + 0x10000;
        addRange(cmap, low, low, rune, 1, 0);
        return;
    }

    if (len > 8)
    {
        return;
    }

    addMrange(cmap, low, values, len);
}

char *parseBfChar(PDFCmapPtr cmap, char *p)
{
    int dst[256];
    int i;
    while (p)
    {
        pair<int, string> next;
        p = nextWord(p, next);
        if (next.first == WORD_ERROR || next.second == "endbfchar")
        {
            return p;
        }
        else if (next.first == WORD_STRING)
        {
            int src = codeFromString(next.second);
            p = nextWord(p, next);
            if (next.first != WORD_STRING)
                return p;
            int tsize = (int)next.second.size() / 2;
            if (tsize)
            {
                int len = tsize < 256 ? tsize : 256;
                for (i = 0; i < len; i++)
                {
                    string temp = next.second.substr(i * 2, 2);
                    dst[i] = codeFromString(temp);
                }
                mapOneToMany(cmap, src, dst, i);
            }
        }
    }

    return p;
}

char *parseCidChar(PDFCmapPtr cmap, char *p)
{
    while (p)
    {
        pair<int, string> next;
        p = nextWord(p, next);
        if (next.first == WORD_ERROR || next.second == "endcidchar")
        {
            return p;
        }
        else if (next.first == WORD_STRING)
        {
            int src = codeFromString(next.second);
            p = nextWord(p, next);
            if (next.first == WORD_OTHER)
            {
                int dst = atoi(next.second.c_str());
                addRange(cmap, src, src, dst, 1, 0);
            }
        }
    }

    return p;
}

char *parseBfRangeArray(PDFCmapPtr cmap, char *p, string &buf, int lo, int hi)
{
    int dst[256];
    int i;
    vector<string> vec;
    getArray(buf, vec);

    for (vector<string>::iterator iter = vec.begin(); iter != vec.end(); ++iter)
    {
        int tsize = iter->size();
        if (tsize / 2)
        {

            int len = tsize < 256 ? tsize : 256;
            for (i = 0; i < len; i++)
            {
                string temp = iter->substr(i * 2, 2);
                dst[i] = codeFromString(temp);
            }
            mapOneToMany(cmap, lo, dst, tsize / 2);
        }
        lo++;
    }
}

char *parseBfRange(PDFCmapPtr cmap, char *p)
{
    while (p)
    {
        pair<int, string> next;
        p = nextWord(p, next);
        if (next.first == WORD_ERROR || next.second == "endbfrange")
        {
            return p;
        }
        else if (next.first == WORD_STRING)
        {
            int lo = codeFromString(next.second);
            p = nextWord(p, next);
            if (next.first == WORD_STRING)
            {
                int hi = codeFromString(next.second);
                if (lo < 0 || lo > 65535 || hi < 0 || hi > 65535 || lo > hi)
                {
                    return p;
                }
                p = nextWord(p, next);
                if (next.first == WORD_STRING)
                {
                    if (next.second.size() == 4)
                    {
                        int dst = codeFromString(next.second);
                        addRange(cmap, lo, hi, dst, 1, 0);
                    }
                    else
                    {
                        int dststr[256];
                        int i;
                        int tsize = (int)next.second.size() / 2;
                        if (tsize)
                        {
                            int len = tsize < 256 ? tsize : 256;
                            for (i = 0; i < len; i++)
                            {
                                string temp = next.second.substr(i * 2, 2);
                                dststr[i] = codeFromString(temp);
                            }
                            while (lo <= hi)
                            {
                                mapOneToMany(cmap, lo, dststr, i);
                                dststr[i - 1]++;
                                lo++;
                            }
                        }
                    }
                }
                else if (next.first == WORD_ARRAY)
                {
                    p = parseBfRangeArray(cmap, p, next.second, lo, hi);
                }
            }
        }
    }

    return p;
}

char *parseCidRange(PDFCmapPtr cmap, char *p)
{
    while (p)
    {
        pair<int, string> next;
        p = nextWord(p, next);
        if (next.first == WORD_ERROR || next.second == "endcidrange")
        {
            return p;
        }
        else if (next.first == WORD_STRING)
        {
            int lo = codeFromString(next.second);
            p = nextWord(p, next);
            if (next.first != WORD_STRING)
                return p;
            int hi = codeFromString(next.second);
            p = nextWord(p, next);
            if (next.first == WORD_OTHER)
            {
                int dst = atoi(next.second.c_str());
                addRange(cmap, lo, hi, dst, 1, 0);
            }
        }
    }

    return p;
}

PDFCmapPtr reMapCmap(PDFCmapPtr encode, PDFCmapPtr unicode)
{
    unsigned int a, b, x;
    int i;
    PDFCmapPtr ucs_from_gid = PDFCmapPtr(new PDFCmap);

    for (i = 0; i < encode->codespace_len; i++)
    {
        addCodeSpace(ucs_from_gid, encode->codespace[i].low, encode->codespace[i].high, encode->codespace[i].n);
    }

    for (i = 0; i < encode->rlen; ++i)
    {
        a = encode->ranges[i].low;
        b = encode->ranges[i].high;
        x = encode->ranges[i].out;

        unsigned int k;
        int ucsbuf[8];
        int ucslen;

        for (k = 0; k <= b - a; ++k)
        {
            ucslen = lookupCmap(unicode, a + k, ucsbuf);
            if (ucslen == 1)
                addRange(ucs_from_gid, x + k, x + k, ucsbuf[0], 1, 0);
            else if (ucslen > 1)
                mapOneToMany(ucs_from_gid, x + k, ucsbuf, ucslen);
        }
    }

    for (i = 0; i < encode->xlen; ++i)
    {
        a = encode->xranges[i].low;
        b = encode->xranges[i].high;
        x = encode->xranges[i].out;
        unsigned int k;
        int ucsbuf[8];
        int ucslen;

        for (k = 0; k <= b - a; ++k)
        {
            ucslen = lookupCmap(unicode, a + k, ucsbuf);
            if (ucslen == 1)
                addRange(ucs_from_gid, x + k, x + k, ucsbuf[0], 1, 0);
            else if (ucslen > 1)
                mapOneToMany(ucs_from_gid, x + k, ucsbuf, ucslen);
        }
    }

    sortCmap(ucs_from_gid);

    return ucs_from_gid;
}

PDFCmapPtr loadEncoding()
{
    PDFCmapPtr encode = PDFCmapPtr(new PDFCmap);
    int wmode = 0;
    int bytes = 1;
    unsigned int high = (1 << (bytes * 8)) - 1;
    if (wmode)
        encode->cmap_name = "Identity-V";
    else
        encode->cmap_name = "Identity-H";

    addCodeSpace(encode, 0, high, bytes);
    addRange(encode, 0, high, 0, 1, 0);
    sortCmap(encode);
    encode->wmode = wmode;

    return encode;
}

PDFCmapPtr loadCmap(char *cUnicodeData)
{
    PDFCmapPtr unicode = PDFCmapPtr(new PDFCmap);
    char *proc = cUnicodeData;
    char *p = proc;
    string key = ".notdef";
    while (p)
    {
        pair<int, string> next;
        p = nextWord(p, next);
        if (next.first == -1)
        {
            break;
        }
        else if (next.first == 0)
        {
            if (next.second == "CMapName")
            {
                p = nextWord(p, next);
                if (next.first == WORD_NAME)
                {
                    unicode->cmap_name = next.second;
                }
            }
            else if (next.second == "WMode")
            {
                p = nextWord(p, next);
                if (next.first != WORD_ERROR)
                {
                    unicode->wmode = atoi(next.second.c_str());
                }
            }
            else
            {
                key = next.second;
            }
        }
        else if (next.first == WORD_OTHER)
        {
            if (next.second == "endcmap")
            {
                break;
            }
            else if (next.second == "usecmap")
            {
                unicode->usecmap_name = key;
            }
            else if (next.second == "begincodespacerange")
            {
                p = parseCodespaceRange(unicode, p);
            }
            else if (next.second == "beginbfchar")
            {
                p = parseBfChar(unicode, p);
            }
            else if (next.second == "begincidchar")
            {
                p = parseCidChar(unicode, p);
            }
            else if (next.second == "beginbfrange")
            {
                p = parseBfRange(unicode, p);
            }
            else if (next.second == "begincidrange")
            {
                p = parseCidRange(unicode, p);
            }
        }
    }
    sortCmap(unicode);
    PDFCmapPtr encode = loadEncoding();
    unicode = reMapCmap(encode, unicode);

    return unicode;
}

int lookupCmap(PDFCmapPtr cmap, unsigned int cpt, int *out)
{
    cmap_range *ranges = cmap->ranges;
    cmap_xrange *xranges = cmap->xranges;
    cmap_mrange *mranges = cmap->mranges;
    unsigned int i;
    int l, r, m;

    l = 0;
    r = cmap->rlen - 1;
    while (l <= r)
    {
        m = (l + r) >> 1;
        if (cpt < ranges[m].low)
            r = m - 1;
        else if (cpt > ranges[m].high)
            l = m + 1;
        else
        {
            out[0] = cpt - ranges[m].low + ranges[m].out;
            return 1;
        }
    }

    l = 0;
    r = cmap->xlen - 1;
    while (l <= r)
    {
        m = (l + r) >> 1;
        if (cpt < xranges[m].low)
            r = m - 1;
        else if (cpt > xranges[m].high)
            l = m + 1;
        else
        {
            out[0] = cpt - xranges[m].low + xranges[m].out;
            return 1;
        }
    }

    l = 0;
    r = cmap->mlen - 1;
    while (l <= r)
    {
        m = (l + r) >> 1;
        if (cpt < mranges[m].low)
            r = m - 1;
        else if (cpt > mranges[m].low)
            l = m + 1;
        else
        {
            int *ptr = &cmap->dict[cmap->mranges[m].out];
            unsigned int len = (unsigned int)*ptr++;
            for (i = 0; i < len; ++i)
                out[i] = *ptr++;
            return len;
        }
    }
    return 0;
}
} // namespace pdf
