/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_herdoc.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jechoi <jechoi@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 15:02:56 by dsagong           #+#    #+#             */
/*   Updated: 2025/09/12 22:34:46 by jechoi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "print.h"

void    print_herdocs(t_hd *hd_lst)
{
	char	buf[1024];
	ssize_t	n;
	t_hd *dup;

	dup = hd_lst;
	if (dup)
	{
		printf("***********HEREDOC LIST**********\n");
		while (dup)
		{
			printf("[heredoc fd: %d]\n", dup->fd);
			while ((n = read(dup->fd, buf, sizeof(buf) - 1)) > 0)
			{
				buf[n] = '\0';
				printf("%s", buf);
			}
			printf("\n----------------------------\n");
			dup = dup->next;
		}
	}
}
