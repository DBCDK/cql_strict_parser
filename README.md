CQLStrictParser
=================

php class for parsing unstrict CQL to strict CQL

This class is able to parse a loose usage of cql to strict cql e.g.:

this is loose and not strict => 'this is loose and not strict'
this is loose AND old NOT strict => 'this is loose' and old not strict
(searccode=some value) AND (searchcode=(value OR another value)) => (searccode='some value') and (searchcode=(value or 'another value'))
