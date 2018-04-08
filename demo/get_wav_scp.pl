#!/usr/bin/perl

#获得预备文件 ./data/test/wav.scp
#保存的是wav文件的编号及其绝对路径
#格式：000001 /home/fhl/test/863/wav/fhl/000001.wav


#脚本功能：获得某绝对路径下，各个speaker目录下的wav文件名（编码）及其绝对路径
if( @ARGV ne 1 )
{
	die "Usage: get_wav_scp.pl dir_absolute\n\nexample: [get_wav_scp.pl /home/fhl/test/863/wav]
";
}

$dir = @ARGV[0];

opendir CURDIR,"$dir" or  die "open cur dir failed!\n";		#open cur dir
@cur_files = readdir CURDIR;		#读入起始目录中所有内容（包括".","..",所有文件及目录）

foreach $cur_file( sort @cur_files )
{
#	print $cur_file."\n";	#include ".","..",all files,all sub_dirs
	if( $cur_file eq "." || $cur_file eq ".." )		#对".","..",及所有文件不处理
	{
	}
	else	#sub dir子目录
	{
		
		@files_dir = <$dir/$cur_file/*.wav>;

		foreach $file_dir( sort @files_dir)
		{
		#	print $file_dir."\n";	#/home/fhl/test/863/wav/fhl/000001.wav
		
			@arr = split "/",$file_dir;
			$filename = pop(@arr);
			chop($filename);
			chop($filename);
			chop($filename);
			chop($filename);
			
			print $filename." ".$file_dir."\n";
		}
	}
}
closedir(CURDIR);

close(F1);
close(F2);
