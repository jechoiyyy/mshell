/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jechoi <jechoi@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/26 20:05:55 by jechoi            #+#    #+#             */
/*   Updated: 2025/09/12 22:30:38 by jechoi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.h"

int	is_redirect_token(t_token_type type)
{
	return (type == T_REDIR_IN || type == T_REDIR_OUT \
		|| type == T_APPEND || type == T_HEREDOC);
}

char	*ft_strncpy(char *dest, const char *src, size_t n)
{
	size_t	i = 0;

	while (i < n && src[i] != '\0')
	{
		dest[i] = src[i];
		i++;
	}

	while (i < n)
	{
		dest[i] = '\0';
		i++;
	}
	return (dest);
}

char	*ft_strcpy(char *dest, const char *src)
{
	int	i;

	i = 0;
	while (src[i] != '\0')
	{
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
	return (dest);
}

char	*ft_strcat(char *dest, const char *src)
{
	int	i;
	int si;

	i = 0;
	si = 0;
	while (dest[i] != '\0')
		i++;
	while (src[si] != '\0')
	{
		dest[i] = src[si];
		i++;
		si++;
	}
	dest[i] = '\0';
	return (dest);	
}

#include <fcntl.h>
#include <errno.h>
#include <string.h>

void check_fd_valid(int fd)
{
    if (fcntl(fd, F_GETFD) == -1)
        printf("FD %d is INVALID (errno=%d: %s)\n", fd, errno, strerror(errno));
    else
        printf("FD %d is VALID\n", fd);
}

void print_fd_content(int fd)
{
	char	buf[1024];
	ssize_t	n;

	printf("***********HEREDOC LIST**********\n");
	printf("[heredoc fd: %d]\n", fd);
	while ((n = read(fd, buf, sizeof(buf) - 1)) > 0)
	{
		buf[n] = '\0';
		printf("%s", buf);
	}
	printf("\n----------------------------\n");
}