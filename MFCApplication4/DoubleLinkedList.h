#include<iostream>
using namespace std;

typedef class Node
{
public:
	int data;
	class Node *next;
}ListNode;

class List
{
public:
	//������������ʼ��(ͷ�巨)
/*	List(int a[], int n){
		ListNode *s;
		int i;
		L = (ListNode *)malloc(sizeof(ListNode));
		L->next = NULL;
		for (i = 0; i < n; i++){
			s = (ListNode *)malloc(sizeof(ListNode));
			s->data = a[i];
			s->next = L->next;
			L->next = s;
		}
	}*/
	//��������ʼ����β�巨��
	List(int a[], int n) {
		ListNode *p, *s;
		int i;
		L = (ListNode *)malloc(sizeof(ListNode));
		p = L;
		for (i = 0; i < n; i++) {
			s = (ListNode *)malloc(sizeof(ListNode));
			s->data = a[i];
			p->next = s;
			p = s;
		}
		p->next = NULL;
	}
	//�������ͷſռ�
	~List() {
		ListNode *pre = L, *p = L->next;
		while (p->next != NULL) {
			free(pre);
			pre = p;
			p = pre->next;
		}
		free(pre);
	}
	//�ж��Ƿ�Ϊ��
	bool isEmpty() {
		return (L->next == NULL);
	}
	//��������
	int getLength() {
		ListNode *p = L;
		int i = 0;
		while (p->next != NULL) {
			i++;
			p = p->next;
		}
		return i;
	}
	//�����������Ԫ��
	void DisplayList() {
		ListNode *p = L->next;//ָ���һ���������ݵĽڵ㣨��ͷ��㣩
		while (p != NULL) {
			cout << p->data << ends;
			p = p->next;
		}
		cout << endl;
	}
	//����Ų��Ҳ���������ֵ
	bool getElem(int i, int &res) {
		ListNode *p = L;
		int j = 0;
		while (j != i && p->next != NULL) {
			j++;
			p = p->next;
		}
		if (j != i && p->next == NULL) {//���뻹��j!=i���������һ���ڵ��޷�����
			return false;
		}
		else {
			res = p->data;
			return true;
		}
	}
	//��ֵ���Ҳ��������
	int LocateElem(int &elem) {
		ListNode *p = L;
		int i = 0;
		while (p->data != elem && p->next != NULL) {
			i++;
			p = p->next;
		}
		if (p->data != elem && p->next == NULL) {
			return 0;
		}
		else {
			return i;
		}
	}
	//����Ԫ��
	void InsertElem(int i, int &elem) {
		i--;
		ListNode *p = L, *s;
		int j = 0;
		while (j != i && p->next != NULL) {
			j++;
			p = p->next;
		}
		if (p->next == NULL) {
			cout << "error number." << endl;
		}
		else {
			s = (ListNode *)malloc(sizeof(ListNode));
			s->data = elem;
			s->next = p->next;
			p->next = s;
		}
	}
	//ɾ��Ԫ��
	bool DeleteElem(int i, int &res) {
		i--;
		ListNode *p = L, *s;
		int j = 0;
		while (j != i && p->next != NULL) {
			j++;
			p = p->next;
		}
		if (p->next == NULL) {
			return false;
		}
		else {
			s = p->next;
			/*	if (s->next == NULL)
					return false;*/
			res = s->data;
			p->next = p->next->next;
			free(s);
			return true;
		}
	}
private:
	Node *L;
	//����ͷ����ַ
};

//int main()
//{
//	//ListNode *m_node;
//	int n, res = 0, a[100], i;
//	cout << "Input the length:" << ends;
//	cin >> n;
//	for (i = 0; i < n; i++) {
//		cin >> a[i];
//	}
//	//����������
//	List m_list(a, n);
//	//���
//	m_list.DisplayList();
//	//�жϿ�,�ǿշ��س���
//	if (!m_list.isEmpty()) {
//		cout << "it is not empty." << endl;
//		cout << "The length:" << m_list.getLength() << endl;
//	}
//	else {
//		cout << "It is empty." << endl;
//	}
//	//����Ų��Ҳ�����Ԫ��
//	cout << "Input number:" << ends;
//	cin >> i;
//	if (m_list.getElem(i, res)) {
//		cout << "NO. " << i << " ,data is " << res << endl;
//	}
//	else {
//		cout << "error number or not contained." << endl;
//	}
//	//��ֵ���Ҳ��������
//	cout << "Input the data:" << ends;
//	cin >> res;
//	if (m_list.LocateElem(res) == 0) {
//		cout << "Not contained." << endl;
//	}
//	else {
//		cout << "The location is No. " << m_list.LocateElem(res) << endl;
//	}
//	//����Ԫ��
//	cout << "Input the number and data:" << ends;
//	cin >> i >> res;
//	m_list.InsertElem(i, res);
//	m_list.DisplayList();
//	//ɾ��Ԫ��
//	cout << "Input number:" << ends;
//	cin >> i;
//	if (m_list.DeleteElem(i, res)) {
//		cout << "The NO. " << i << " ,data: " << res << " is deleted." << endl;
//		m_list.DisplayList();
//	}
//	else {
//		cout << "Not contain this number." << endl;
//	}
//	return 0;
//}