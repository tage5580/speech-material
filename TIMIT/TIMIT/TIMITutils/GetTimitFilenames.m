function fnames=GetTimitFilenames(gender)
%
% fnames=GetTimitFilenames(gender)
%
% where 'gender' is 'f' or 'm'
%
% Aki Harma, 5-6-2005, Philips Research

if ispc,
    Tpath = '..';
   % Tpath='H:\Audio\Speech\databases\TIMIT\timit\train';
else
    Tpath='/home/nlv11251/Audio/Speech/databases/TIMIT/timit/train';
end

dd1=dir([Tpath '/d*']);dd1=char(dd1.name);

fnames=char(' ');
Nd1=size(dd1,1);
for q=1:Nd1,
    Lpath=[Tpath '/' dd1(q,:)];
    dd2=dir([Lpath '/' gender '*']); dd2=char(dd2.name);
    Nd2=size(dd2,1);
    for w=1:Nd2,
        Mpath=[Lpath '/' deblank(dd2(w,:))];
        dd3=dir([Mpath '/*.wav']);dd3=char(dd3.name); 
        for e=1:size(dd3,1),
            fnames=char(fnames,[Mpath '/' deblank(dd3(e,:))]);
        end
    end
end

fnames(1,:)=[];

   