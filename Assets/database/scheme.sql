drop table shader;

create table if not exists shader(
	id integer not null primary key,
	path text not null,
	data blob
);

insert into shader (id, path, data) values (1, 'effa', 'efafeafa');
select * from shader;