<?php

/**
 * Class CQLStrictParser
 *
 * This class is able to parse a loose usage of cql to strict cql e.g.:
 *
 * this is loose and not strict => "this is loose and not strict"
 * this is loose AND old NOT strict => "this is loose" and "old" not "strict"
 * (searccode=some value) AND (searchcode=(value OR another value)) =>
 *    (searccode='some value') and (searchcode=("value" or "another value"))
 *
 */
class CQLStrictParser {

  private $operators = array(
    '@ IKKE @' => ' not ',
    '@ NOT @' => ' not ',
    '@ ELLER @' => ' or ',
    '@ OR @' => ' or ',
    '@ OG @' => ' and ',
    '@ AND @' => ' and ',
  );
  /** Regular expression for extracting sub-expressions
   * @var string
   */
  private $sub_regex = '/(\((?>[^()]+|(?1))*\))/';

  /** Takes a loose formatted cql string and parses to strict cql
   * @param $expression
   * @return string
   */
  public function parse($expression) {
    $expression = $this->parseSubExpression($expression);
    $expression = $this->replaceOperators($expression);
    return $expression;
  }

  /** Extracts sub-expressions (expressions encapsulated by () ) and converts to
   * strict cql
   * @param $expression
   * @return string
   */
  private function parseSubExpression($expression) {
    preg_match_all($this->sub_regex, $expression, $match);

    if (!empty($match[1])) {
      foreach ($match[1] as $needle) {
        $trimmed_needle = $this->trimString($needle);
        $replacement = $this->parseSubExpression($trimmed_needle);
        $expression = str_replace($needle, '(' . $replacement .')', $expression);
      }
    }

    return $this->parseExpressionsInSubExpressions($expression);
  }

  /** Convert expressions in sub-expressions to strict cql
   * @param $expression
   * @return string
   */
  private function parseExpressionsInSubExpressions($expression) {
    $sub = preg_replace($this->sub_regex, '[SUB]', $expression);
    $expressions = explode('[SUB]', $sub);
    usort($expressions, 'self::sortStringsByLength');
    foreach ($expressions as $needle) {
      $replacement = $this->parseExpressions($needle);
      $expression = str_replace($needle, $replacement, $expression);
    }

    return $expression;

  }

  /** Replace loose operators with strict operators
   * @param $expression
   * @return string
   */
  private function replaceOperators($expression) {
    return preg_replace(array_keys($this->operators), array_values($this->operators), $expression);
  }

  /** Converts an expression without subexpressions to strict cql
   * @param $expression
   * @return mixed
   */
  private function parseExpressions($expression) {

    // Extract searchCode/value expressions
    $expressions = $this->getSearchCodes($expression);
    $tokens = array();

    // Loop through expressions replace with strict cql
    foreach ($expressions as $needle) {
      $replacement = $this->parseSearchCode($needle);
      if(!empty($replacement)){
        $token = '[token ' . count($tokens) .']';
        $tokens[$token] = $replacement;
        $expression = str_replace($needle, $token, $expression);
      }
    }

    return str_replace(array_keys($tokens), array_values($tokens), $expression);
  }

  /** Explodes expressions by operators and returns them sorted by lenght
   * @param $expression
   * @return array
   */
  private function getSearchCodes($expression) {
    $operators = array_keys($this->operators);
    $expressions_with_op = preg_replace($operators, '[OP]', $expression);
    $expressions = explode('[OP]', $expressions_with_op);
    usort($expressions, 'self::sortStringsByLength');
    return $expressions;
  }

  /** converts a key=value pair to strict cql
   * @param $expression
   * @return string
   */
  private function parseSearchCode($expression) {
    if (empty($expression))
      return $expression;
    $expression = trim($expression);
    if (strpos($expression, '=') !== false) {
      list($searchcode, $searchvalue) = explode('=', $expression);
      if (!empty($searchvalue) && strpos($searchvalue, ' ') !== FALSE) {
        $searchvalue = "\"$searchvalue\"";
      }
      $expression = $searchcode . "=" . $searchvalue;
    }
    else if (strpos($expression, ' ') !== FALSE){
      $expression = "\"$expression\"";
    }
    return $expression;
  }

  /** Order strings by length
   * @param $a
   * @param $b
   * @return int
   */
  private function sortStringsByLength($a, $b) {
    return strlen($b) - strlen($a);
  }

  /** Remove defined beginning and ending characters
   * @param $expression
   * @param string $first
   * @param string $last
   * @return string
   */
  private function trimString($expression, $first = '(', $last = ')') {
    if (substr($expression, 0, 1) === $first){
      $expression = substr($expression, 1);
    }
    if (substr($expression, -1, 1) === $last){
      $expression = substr($expression, 0, -1);
    }
    return $expression;
  }

}
