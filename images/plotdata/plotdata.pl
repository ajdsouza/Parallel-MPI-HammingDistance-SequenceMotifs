#/usr/bin/perl


use strict;
use warnings;

use File::Spec;

my %ld;
my @results;
my @errors;

die " provide directory path as arg1 perl plotgraph <dir,dir,dir>\n" unless @ARGV > 0;

my $dirs = $ARGV[0];
my @dirs = split/,/,$dirs;

for my $dir ( @dirs ) {

opendir(DIR, $dir) or die $!;

my @files 
        = grep { 
            /^p\-\d+\-dp\-\d+\-fn\-\d+_\d+_\d+\.txt/             # p-16-dp-4-fn-300_33_15.txt
	    && -f "$dir/$_"   # and is a file
	} readdir(DIR);

closedir(DIR);

# Loop through the array printing out the filenames

     foreach my $file (@files) {
       
        $file =~ s/\s+// if $file;

        my ($p,$k,$n,$l,$d) = ( $file =~ /^p\-(\d+)\-dp\-(\d+)\-fn\-(\d+)_(\d+)_(\d+)\.txt/ );

        open (FH, File::Spec->catfile($dir,$file)) || die "ERROR Unable to open file: $!\n";

	my $last;
	my $first = <FH>;

	while (<FH>) { $last = $_ }
	close FH;

        $last =~ s/\s+// if $last;
        chomp $last if $last;
        $last =~ s/\s+// if $last;

        my $msg = '';

        unless ($last and $last =~ /^\d+.\d+$/) {
         $msg = $last if $last;
         $last = -1;
         push @errors,"$file=$p,$k,$n,$l,$d=$last,\"$msg\"";
	if ( $p > 1 ) {
         push @{$ld{nld}{"n=$n-l=$l-d=$d"}{$p}{$k}},$last;
         push @{$ld{pld}{"p=$p-l=$l-d=$d"}{$n}{$k}},$last;
         push @{$ld{kld}{"k=$k-l=$l-d=$d"}{$p}{$n}},$last;
         push @{$ld{kpn}{"k=$k-p=$p-n=$n"}{$l}{$d}},$last;
	 push @{$ld{pdn}{"p=$p-d=$d-n=$n"}{$k}{$l}},$last;
        } else {
         push @{$ld{d}{"d=$d"}{$l}{$n}},$last;
         push @{$ld{l}{"l=$l"}{$d}{$n}},$last;
        }
        }
        else {
         push @results, "$file=$p,$k,$n,$l,$d=$last,\"$msg\"";
	if ( $p > 1 ) {
         push @{$ld{nld}{"n=$n-l=$l-d=$d"}{$p}{$k}},$last;
         push @{$ld{pld}{"p=$p-l=$l-d=$d"}{$n}{$k}},$last;
         push @{$ld{kld}{"k=$k-l=$l-d=$d"}{$p}{$n}},$last;
         push @{$ld{kpn}{"k=$k-p=$p-n=$n"}{$l}{$d}},$last;
 	 push @{$ld{pdn}{"p=$p-d=$d-n=$n"}{$k}{$l}},$last;
        } else {
         push @{$ld{d}{"d=$d"}{$l}{$n}},$last;
         push @{$ld{l}{"l=$l"}{$d}{$n}},$last;
        }
        }


    }

}

my $plot = "gplot.txt";
open(FH, '>>', $plot) or die "Failed to open file $plot for writing \n";

for my $res (@results){
   print FH "$res\n";
}
for my $err (@errors){
   print FH "$err\n";
}
close(FH);


my %tags = ('nld'=>'p','pld'=>'n','kld'=>'p','kpn'=>'l','d'=>'l','l'=>'d','pdn'=>'k');

for my $type ( sort keys %ld ){

   my @pso;
   for my $nld ( sort keys %{$ld{$type}} ) {
     push @pso,keys %{$ld{$type}{$nld}};
   }
   my %psoh = map { $_ => 1 } @pso ;

   my $plotnld = "gplot$type.txt";
   open(FH, '>>', $plotnld) or die "Failed to open file $plotnld for writing \n";

       
 for my $nld ( sort keys %{$ld{$type}}){
   
  print FH "$type,";
  print FH ",";

  for my $p (  sort { $a <=> $b} keys %psoh  ){
   print FH "$tags{$type}=$p,";
  }
  print FH "\n";


   my @kso;
   for my $p ( sort keys %{$ld{$type}{$nld}} ) {
     push @kso,keys %{$ld{$type}{$nld}{$p}};
   }

   my %ksoh = map { $_ => 1 } @kso ;

  for my $k ( sort {$a <=> $b} keys %ksoh ){
      
    print FH "$nld,$k,";
    
    for my $p ( sort { $a <=> $b} keys %psoh ){

     if ( $ld{$type}{$nld}{$p}{$k} and @{$ld{$type}{$nld}{$p}{$k}}){
      for my $tm ( sort @{$ld{$type}{$nld}{$p}{$k}}){
       print FH "$tm,";
       last;
      }
     }
     else {
       print FH ",";
     }

   }
   print FH "\n";
  }


 }
     close(FH);
}





exit 0;
