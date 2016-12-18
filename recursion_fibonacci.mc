/* 재귀 함수로 구현한 fibonacci 함수 */

void main()
{
	int i;
	i = 2;
	while(i <= 9) {
		write ( fibonacci(i) );
		i++;
	}
}

int fibonacci(int n)
{
	if( n <= 1 )
		return n;
	else
		return (fibonacci(n-1) + fibonacci(n-2));
}
