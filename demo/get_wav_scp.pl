#!/usr/bin/perl

#���Ԥ���ļ� ./data/test/wav.scp
#�������wav�ļ��ı�ż������·��
#��ʽ��000001 /home/fhl/test/863/wav/fhl/000001.wav


#�ű����ܣ����ĳ����·���£�����speakerĿ¼�µ�wav�ļ��������룩�������·��
if( @ARGV ne 1 )
{
	die "Usage: get_wav_scp.pl dir_absolute\n\nexample: [get_wav_scp.pl /home/fhl/test/863/wav]
";
}

$dir = @ARGV[0];

opendir CURDIR,"$dir" or  die "open cur dir failed!\n";		#open cur dir
@cur_files = readdir CURDIR;		#������ʼĿ¼���������ݣ�����".","..",�����ļ���Ŀ¼��

foreach $cur_file( sort @cur_files )
{
#	print $cur_file."\n";	#include ".","..",all files,all sub_dirs
	if( $cur_file eq "." || $cur_file eq ".." )		#��".","..",�������ļ�������
	{
	}
	else	#sub dir��Ŀ¼
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
