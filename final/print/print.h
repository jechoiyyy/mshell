/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jechoi <jechoi@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/04 17:49:06 by dsagong           #+#    #+#             */
/*   Updated: 2025/09/16 12:35:46 by jechoi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PRINT_H
# define PRINT_H
# include "types.h"
# include <stdio.h>

//void	print_tokens(t_token *tokens, char *step);
//void	print_herdocs(t_hd *hd_lst);
void	print_cmd_list(t_cmd *head);

#endif
